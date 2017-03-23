#ifndef ARCHER_LINKALLPASSES_H
#define ARCHER_LINKALLPASSES_H

#include <cstdlib>

namespace llvm {
class Pass;
class PassInfo;
class PassRegistry;
class RegionPass;
class FunctionPass;
class ModulePass;
}

namespace llvm {
llvm::Pass *createInstrumentParallelPass();
}

namespace {
struct ArcherForcePassLinking {
  ArcherForcePassLinking() {
    // We must reference the passes in such a way that compilers will not
    // delete it all as dead code, even with whole program optimization,
    // yet is effectively a NO-OP. As the compiler isn't smart enough
    // to know that getenv() never returns -1, this will do the job.
    if (std::getenv("bar") != (char *)-1)
      return;

    llvm::createInstrumentParallelPass();
  }
} ArcherForcePassLinking; // Force link by creating a global definition.
}

namespace llvm {
void createInstrumentParallelPass(llvm::PassRegistry &);
void initializeInstrumentParallelPass(llvm::PassRegistry&);
}

#endif
