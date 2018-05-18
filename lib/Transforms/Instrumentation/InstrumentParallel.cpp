/*
Copyright (c) 2015-2017, Lawrence Livermore National Security, LLC.

Produced at the Lawrence Livermore National Laboratory

Written by Simone Atzeni (simone@cs.utah.edu), Joachim Protze
(joachim.protze@tu-dresden.de), Jonas Hahnfeld
(hahnfeld@itc.rwth-aachen.de), Ganesh Gopalakrishnan, Zvonimir
Rakamaric, Dong H. Ahn, Gregory L. Lee, Ignacio Laguna, and Martin
Schulz.

LLNL-CODE-727057

All rights reserved.

This file is part of Archer. For details, see
https://pruners.github.io/archer. Please also read
https://github.com/PRUNERS/archer/blob/master/LICENSE.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

   Redistributions of source code must retain the above copyright
   notice, this list of conditions and the disclaimer below.

   Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the disclaimer (as noted below)
   in the documentation and/or other materials provided with the
   distribution.

   Neither the name of the LLNS/LLNL nor the names of its contributors
   may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE
LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "llvm/Transforms/Instrumentation.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/CaptureTracking.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/ProfileData/InstrProf.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/EscapeEnumerator.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "archer/LinkAllPasses.h"

using namespace llvm;

#define MIN_VERSION 39

namespace {

struct InstrumentParallel : public FunctionPass {
  InstrumentParallel() : FunctionPass(ID) { PassName = "InstrumentParallel"; }
#if LLVM_VERSION > MIN_VERSION
  StringRef getPassName() const override;
#else
  const char *getPassName() const override;
#endif
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnFunction(Function &F) override;
  bool doInitialization(Module &M) override;
  static char ID;  // Pass identification, replacement for typeid.

private:
  std::string PassName;
  void setMetadata(Instruction *Inst, const char *name, const char *description);
};
}  // namespace

char InstrumentParallel::ID = 0;
INITIALIZE_PASS_BEGIN(
    InstrumentParallel, "archer-sbl",
    "InstrumentParallel: instrument parallel functions.",
    false, false)
INITIALIZE_PASS_END(
    InstrumentParallel, "archer-sbl",
    "InstrumentParallel: instrument parallel functions.",
    false, false)

#if LLVM_VERSION > MIN_VERSION
	StringRef InstrumentParallel::getPassName() const {
		return PassName;
	}
#else
	const char *InstrumentParallel::getPassName() const {
		return PassName.c_str();
	}
#endif

void InstrumentParallel::getAnalysisUsage(AnalysisUsage &AU) const {
}

Pass *llvm::createInstrumentParallelPass() {
  return new InstrumentParallel();
}

bool InstrumentParallel::doInitialization(Module &M) {
  return true;
}

void InstrumentParallel::setMetadata(Instruction *Inst, const char *name, const char *description) {
  LLVMContext& C = Inst->getContext();
  MDNode* N = MDNode::get(C, MDString::get(C, description));
  Inst->setMetadata(name, N);
}

bool InstrumentParallel::runOnFunction(Function &F) {
  llvm::GlobalVariable *ompStatusGlobal = NULL;
  Module *M = F.getParent();
  StringRef functionName = F.getName();

  ConstantInt *Zero = ConstantInt::get(Type::getInt32Ty(M->getContext()), 0);
  ConstantInt *One = ConstantInt::get(Type::getInt32Ty(M->getContext()), 1);

  ompStatusGlobal = M->getNamedGlobal("__archer_status__");
  if(functionName.compare("main") == 0) {
    if(!ompStatusGlobal) {
      IntegerType *Int32Ty = IntegerType::getInt32Ty(M->getContext());
      ompStatusGlobal =
        new llvm::GlobalVariable(*M, Int32Ty, false,
                                 llvm::GlobalValue::CommonLinkage,
                                 Zero, "__archer_status__", NULL,
                                 GlobalVariable::GeneralDynamicTLSModel,
                                 0, false);
    } else if(ompStatusGlobal &&
              (ompStatusGlobal->getLinkage() != llvm::GlobalValue::CommonLinkage)) {
      ompStatusGlobal->setLinkage(llvm::GlobalValue::CommonLinkage);
      ompStatusGlobal->setExternallyInitialized(false);
      ompStatusGlobal->setInitializer(Zero);
    }

#if !LIBOMP_TSAN_SUPPORT
    // Add function for Tsan suppressions
    // const char *__tsan_default_suppressions() {
    //   return "called_from_lib:libomp.so\nthread:^__kmp_create_worker$\n";
    // }
    Constant *suppression_str_const =
      ConstantDataArray::getString(M->getContext(),
				   "called_from_lib:libomp.*\nthread:^__kmp_create_worker$\n", true);
    GlobalVariable* suppression_str =
      new GlobalVariable(*M,
                         suppression_str_const->getType(),
                         true,
                         GlobalValue::PrivateLinkage,
                         suppression_str_const,
                         "__tsan_default_suppressions_value");
    suppression_str->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
    suppression_str->setAlignment(1);
    IRBuilder<> IRB(M->getContext());
    Function* __tsan_default_suppressions = cast<Function>(M->getOrInsertFunction("__tsan_default_suppressions",
										  IRB.getInt8PtrTy()));
    __tsan_default_suppressions->setCallingConv(CallingConv::C);
    __tsan_default_suppressions->addFnAttr(Attribute::NoInline);
    __tsan_default_suppressions->addFnAttr(Attribute::NoUnwind);
    __tsan_default_suppressions->addFnAttr(Attribute::OptimizeNone);
    __tsan_default_suppressions->addFnAttr(Attribute::UWTable);
    __tsan_default_suppressions->setUnnamedAddr(GlobalValue::UnnamedAddr::None);
    BasicBlock* block = BasicBlock::Create(M->getContext(), "entry", __tsan_default_suppressions);
    IRBuilder<> builder(block);
    Value* indexList[2] = { ConstantInt::get(builder.getInt32Ty(), 0), ConstantInt::get(builder.getInt32Ty(), 0) };
    builder.CreateRet(builder.CreateGEP(suppression_str, indexList, ""));
#endif

#if LLVM_VERSION >= 40
    IRBuilder<> IRB2(M->getContext());
    Constant* constant = M->getOrInsertFunction("__archer_get_omp_status",
    		IRB2.getInt32Ty());
    Function* __archer_get_omp_status = cast<Function>(constant);
    __archer_get_omp_status->setCallingConv(CallingConv::C);
    BasicBlock* block2 = BasicBlock::Create(M->getContext(), "entry", __archer_get_omp_status);
    IRBuilder<> builder2(block2);
    LoadInst *loadOmpStatus = builder2.CreateLoad(IRB2.getInt32Ty(), ompStatusGlobal);
    builder2.CreateRet(loadOmpStatus);
#endif

    F.removeFnAttr(llvm::Attribute::SanitizeThread);
    return true;
  }

  if(functionName.endswith("_dtor") ||
     functionName.endswith("__archer__") ||
     functionName.endswith("__clang_call_terminate") ||
     functionName.endswith("__tsan_default_suppressions") ||
     functionName.endswith("__archer_get_omp_status") ||
     (F.getLinkage() == llvm::GlobalValue::AvailableExternallyLinkage)) {
    return true;
  }

  if(!ompStatusGlobal) {
    IntegerType *Int32Ty = IntegerType::getInt32Ty(M->getContext());
    ompStatusGlobal =
      new llvm::GlobalVariable(*M, Int32Ty, false,
                               llvm::GlobalValue::AvailableExternallyLinkage,
                               0, "__archer_status__", NULL,
                               GlobalVariable::GeneralDynamicTLSModel,
                               0, true);
  }

  if(functionName.startswith(".omp")) {
    // Increment of __archer_status__
    Instruction *entryBBI = &F.getEntryBlock().front();
    LoadInst *loadInc = new LoadInst(ompStatusGlobal, "loadIncOmpStatus", false, entryBBI);
    loadInc->setAlignment(4);
    setMetadata(loadInc, "archer.ompstatus", "ArcherRT Instrumentation");
    Instruction *inc = BinaryOperator::Create (BinaryOperator::Add,
                                               loadInc, One,
                                               "incOmpStatus",
                                               entryBBI);
    setMetadata(loadInc, "archerrt.ompstatus", "ArcherRT Instrumentation");
    StoreInst *storeInc = new StoreInst(inc, ompStatusGlobal, entryBBI);
    storeInc->setAlignment(4);
    setMetadata(storeInc, "archerrt.ompstatus", "ArcherRT Instrumentation");

    // Decrement of __archer_status__
    Instruction *exitBBI = F.back().getTerminator();
    if(exitBBI) {
      LoadInst *loadDec = new LoadInst(ompStatusGlobal, "loadDecOmpStatus", false, exitBBI);
      loadDec->setAlignment(4);
      setMetadata(loadDec, "archerrt.ompstatus", "ArcherRT Instrumentation");
      Instruction *dec = BinaryOperator::Create (BinaryOperator::Sub,
                                                 loadDec, One,
                                                 "decOmpStatus",
                                                 exitBBI);
      StoreInst *storeDec = new StoreInst(dec, ompStatusGlobal, exitBBI);
      storeDec->setAlignment(4);
      setMetadata(storeDec, "archerrt.ompstatus", "ArcherRT Instrumentation");
    } else {
      report_fatal_error("Broken function found, compilation aborted!");
    }
  } else {
    ValueToValueMapTy VMap;
    Function *new_function = CloneFunction(&F, VMap);
    new_function->setName(functionName + "__archer__");
    Function::arg_iterator it = F.arg_begin();
    Function::arg_iterator end = F.arg_end();
    std::vector<Value*> args;
    while (it != end) {
      Argument *Args = &(*it);
      args.push_back(Args);
      it++;
    }

    // Removing SanitizeThread attribute so the sequential functions
    // won't be instrumented
    F.removeFnAttr(llvm::Attribute::SanitizeThread);

    // Instruction *firstEntryBBI = &F.getEntryBlock().front();
    Instruction *firstEntryBBI = NULL;
    Instruction *firstEntryBBDI = NULL;
    // for (auto &BB : F) {
    for (auto &Inst : F.getEntryBlock()) {
      if(Inst.getDebugLoc()) {
        firstEntryBBDI = &Inst;
        break;
      }
    }
    firstEntryBBI = &F.getEntryBlock().front();
    // }
    if(!firstEntryBBI || !firstEntryBBI)
      report_fatal_error("No instructions with debug information!");

    LoadInst *loadOmpStatus = new LoadInst(ompStatusGlobal, "loadOmpStatus", false, firstEntryBBI);
    Instruction *CondInst = new ICmpInst(firstEntryBBI, ICmpInst::ICMP_EQ, loadOmpStatus, One, "__archer__cond");

    BasicBlock *newEntryBB = F.getEntryBlock().splitBasicBlock(firstEntryBBI, "__archer__entry");
    F.getEntryBlock().back().eraseFromParent();
    BasicBlock *archerThenBB = BasicBlock::Create(M->getContext(), "__archer__if.then", &F);
    BranchInst::Create(archerThenBB, newEntryBB, CondInst, &F.getEntryBlock());

    // For now we assing the debug loc of the first instruction of the
    // cloned function
    if(new_function->getReturnType()->isVoidTy()) {
      CallInst *parallelCall = CallInst::Create(new_function, args, "", archerThenBB);
      parallelCall->setDebugLoc(firstEntryBBDI->getDebugLoc());
      ReturnInst::Create(M->getContext(), nullptr, archerThenBB);
    } else {
      CallInst *parallelCall = CallInst::Create(new_function, args, functionName + "__archer__", archerThenBB);
      parallelCall->setDebugLoc(firstEntryBBDI->getDebugLoc());
      ReturnInst::Create(M->getContext(), parallelCall, archerThenBB);
    }
 }

  return true;
}

static void registerInstrumentParallelPass(const PassManagerBuilder &, llvm::legacy::PassManagerBase &PM) {
  PM.add(new InstrumentParallel());
}

static RegisterStandardPasses RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible, registerInstrumentParallelPass);
