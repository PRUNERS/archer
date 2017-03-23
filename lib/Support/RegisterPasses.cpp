#include "archer/LinkAllPasses.h"
#include "llvm/Analysis/CFGPrinter.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Vectorize.h"

namespace llvm {
void initializeArcherPasses(llvm::PassRegistry &Registry) {
  initializeInstrumentParallelPass(Registry);
}

void registerArcherPasses(llvm::legacy::PassManagerBase &PM) {
  PM.add(createInstrumentParallelPass());
}
}
