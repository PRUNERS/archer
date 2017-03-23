#ifndef ARCHER_REGISTER_PASSES_H
#define ARCHER_REGISTER_PASSES_H

#include "llvm/IR/LegacyPassManager.h"

namespace llvm {
  namespace legacy {
    class PassManagerBase;
  }

  void initializeArcherPasses(llvm::PassRegistry &Registry);
  void registerArcherPasses(llvm::legacy::PassManagerBase &PM);
}
#endif
