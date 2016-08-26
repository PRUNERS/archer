//===------ RegisterPasses.cpp - Add the Archer Passes to default passes  --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file composes the individual LLVM-IR passes provided by Archer to a
// functional polyhedral optimizer. The polyhedral optimizer is automatically
// made available to LLVM based compilers by loading the Archer shared library
// into such a compiler.
//
// The Archer optimizer is made available by executing a static constructor that
// registers the individual Archer passes in the LLVM pass manager builder. The
// passes are registered such that the default behaviour of the compiler is not
// changed, but that the flag '-polly' provided at optimization level '-O3'
// enables additional polyhedral optimizations.
//===----------------------------------------------------------------------===//

#include "archer/LinkAllPasses.h"
#include "llvm/Analysis/CFGPrinter.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Vectorize.h"

using namespace llvm;

namespace archer {
void initializeArcherPasses(llvm::PassRegistry &Registry) {
  initializeInstrumentParallelPass(Registry);
}

void registerArcherPasses(llvm::legacy::PassManagerBase &PM) {
  PM.add(createInstrumentParallelPass());
}
}
