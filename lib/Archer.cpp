#include "archer/RegisterPasses.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Support/raw_ostream.h"

namespace {

/// @brief Initialize Archer passes when library is loaded.
///
/// We use the constructor of a statically declared object to initialize the
/// different Archer passes right after the Archer library is loaded. This ensures
/// that the Archer passes are available e.g. in the 'opt' tool.
class StaticInitializer {
public:
  StaticInitializer() {
    llvm::PassRegistry &Registry = *llvm::PassRegistry::getPassRegistry();
    llvm::initializeArcherPasses(Registry);
  }
};
static StaticInitializer InitializeEverything;
} // end of anonymous namespace.
