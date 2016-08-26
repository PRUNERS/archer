//===-- InstrumentParallel.cpp - Archer race detector -------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of Archer, an OpenMP race detector.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Instrumentation.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/CaptureTracking.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/ProfileData/InstrProf.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "archer/LinkAllPasses.h"

using namespace llvm;

#define DEBUG_TYPE "archer"

namespace {

struct InstrumentParallel : public FunctionPass {
  InstrumentParallel() : FunctionPass(ID) {}
  const char *getPassName() const override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnFunction(Function &F) override;
  bool doInitialization(Module &M) override;
  static char ID;  // Pass identification, replacement for typeid.
};
}  // namespace

char InstrumentParallel::ID = 0;
static void *initializeInstrumentParallelPassOnce(PassRegistry &Registry) {
  PassInfo *PI = new PassInfo(
                              "InstrumentParallel: instrument parallel functions.", "archer-sbl", &InstrumentParallel::ID,
                              PassInfo::NormalCtor_t(callDefaultCtor<InstrumentParallel>), false, false);
  Registry.registerPass(*PI, true);
  return PI;
}

LLVM_DEFINE_ONCE_FLAG(InitializeInstrumentParallelPassFlag);

void llvm::initializeInstrumentParallelPass(PassRegistry &Registry) {
  llvm::call_once(InitializeInstrumentParallelPassFlag,
                  initializeInstrumentParallelPassOnce, std::ref(Registry));
}

const char *InstrumentParallel::getPassName() const {
  return "InstrumentParallel";
}

void InstrumentParallel::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<TargetLibraryInfoWrapperPass>();
}

Pass *llvm::createInstrumentParallelPass() {
  return new InstrumentParallel();
}

bool InstrumentParallel::doInitialization(Module &M) {
  return true;
}

bool InstrumentParallel::runOnFunction(Function &F) {
  // bool SanitizeFunction = F.hasFnAttribute(Attribute::SanitizeThread);
  // const DataLayout &DL = F.getParent()->getDataLayout();

  StringRef functionName = F.getName();
  
  // if(SanitizeFunction)
  //   printf("%s is a sanitize function\n", functionName.str().c_str());

  if((functionName.compare("main") == 0) ||
     (functionName.endswith("__swordomp__"))) {
    return true;
  }

  Module *M = F.getParent();
  ConstantInt *One = ConstantInt::get(Type::getInt32Ty(M->getContext()), 1);

  // Insert in TLS extern variable
  // extern __thread int __swordomp_status__;
  llvm::GlobalVariable *ompStatusGlobal;
  ompStatusGlobal = M->getNamedGlobal("__swordomp_status__");
  if(!ompStatusGlobal) {
    IntegerType *Int32Ty = IntegerType::getInt32Ty(M->getContext());
    ompStatusGlobal = new llvm::GlobalVariable(*M, Int32Ty, false,
                             llvm::GlobalValue::ExternalLinkage, 0,
                             "__swordomp_status__", NULL,
                             GlobalVariable::GeneralDynamicTLSModel, 0, true);
    ompStatusGlobal->setAlignment(4);
  }
  
  if(functionName.startswith(".omp_outlined")) {
    // Increment of __swordomp_status__
    Instruction *entryBBI = &F.getEntryBlock().front();
    LoadInst *loadInc = new LoadInst(ompStatusGlobal, "loadIncOmpStatus", false, entryBBI);
    loadInc->setAlignment(4);
    Instruction *inc = BinaryOperator::Create (BinaryOperator::Add,
                                               loadInc, One,
                                               "incOmpStatus",
                                               entryBBI);
    StoreInst *storeInc = new StoreInst(inc, ompStatusGlobal, entryBBI);
    storeInc->setAlignment(4);

    // Decrement of __swordomp_status__
    Instruction *exitBBI = F.back().getTerminator();
    if(exitBBI) {
      LoadInst *loadDec = new LoadInst(ompStatusGlobal, "loadDecOmpStatus", false, exitBBI);
      Instruction *dec = BinaryOperator::Create (BinaryOperator::Sub,
                                                 loadDec, One,
                                                 "decOmpStatus",
                                                 exitBBI);
      new StoreInst(dec, ompStatusGlobal, exitBBI);
    } else {
      report_fatal_error("Broken function found, compilation aborted!");
    }
  } else {
    ValueToValueMapTy VMap;
    Function *new_function = CloneFunction(&F, VMap);
    new_function->setName(functionName + "__swordomp__");
    Function::ArgumentListType::iterator it = F.getArgumentList().begin();                                                       Function::ArgumentListType::iterator end = F.getArgumentList().end();
    std::vector<Value*> args;
    while (it != end) {
      Argument *Args = &(*it);
      args.push_back(Args);
      it++;                                                                                                                      }

    // Removing SanitizeThread attribute so the sequential functions
    // won't be instrumented
    F.removeFnAttr(llvm::Attribute::SanitizeThread);
    // bool SanitizeFunction = F.hasFnAttribute(Attribute::SanitizeThread);
    // if(SanitizeFunction)
    //   printf("Function %s won't be sanitized\n", functionName.str().c_str());

    Instruction *firstEntryBBI = &F.getEntryBlock().front();
    LoadInst *loadOmpStatus = new LoadInst(ompStatusGlobal, "", false, firstEntryBBI);
    loadOmpStatus->setAlignment(4);
    Instruction *CondInst = new ICmpInst(firstEntryBBI, ICmpInst::ICMP_EQ, loadOmpStatus, One, "__swordomp__cond");
    BasicBlock *newEntryBB = F.getEntryBlock().splitBasicBlock(firstEntryBBI, "__swordomp__entry");
    F.getEntryBlock().back().eraseFromParent();
    BasicBlock *swordThenBB = BasicBlock::Create(M->getContext(), "__swordomp__if.then", &F);
    BranchInst::Create(swordThenBB, newEntryBB, CondInst, &F.getEntryBlock());
    
    if(new_function->getReturnType()->isVoidTy()) {
      CallInst::Create(new_function, args, "", swordThenBB);
      ReturnInst::Create(M->getContext(), nullptr, swordThenBB);
    } else {
      CallInst *parallelCall = CallInst::Create(new_function, args, functionName + "__swordomp__", swordThenBB);
      ReturnInst::Create(M->getContext(), parallelCall, swordThenBB);
    }
  }

  // Traverse all instructions, collect loads/stores/returns, check for calls.
  // for (auto &BB : F) {
  //   for (auto &Inst : BB) {
  //   }
  // }
  return true;
}

static void registerInstrumentParallelPass(const PassManagerBuilder &, llvm::legacy::PassManagerBase &PM) {
  PM.add(new InstrumentParallel());
}

static RegisterStandardPasses RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible, registerInstrumentParallelPass);
