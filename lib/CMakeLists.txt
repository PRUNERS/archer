set(LLVM_NO_RTTI 1)

set(ARCHER_HEADER_FILES)
if (MSVC_IDE OR XCODE)
  file(GLOB_RECURSE ARCHER_HEADER_FILES "${ARCHER_SOURCE_DIR}/include/archer/*.h")
endif ()

add_archer_library(Archer
  Transforms/Instrumentation/InstrumentParallel.cpp
  Support/RegisterPasses.cpp
  Support/Util.cpp
  ${ARCHER_HEADER_FILES}
  )

target_link_libraries(Archer)

if (BUILD_SHARED_LIBS)
  target_link_libraries(Archer
    LLVMSupport
    LLVMCore
    LLVMScalarOpts
    LLVMInstCombine
    LLVMTransformUtils
    LLVMAnalysis
    LLVMipo
    LLVMMC
  )
  link_directories(
    ${LLVM_LIBRARY_DIR}
  )
endif()

# Build a monolithic Archer.a and a thin module LLVMArcher.moduleext that links to
# that static library.
if (MSVC)
  # Add dummy target, because loadable modules are not supported on Windows
  add_custom_target(LLVMArcher)
  set_target_properties(LLVMArcher PROPERTIES FOLDER "Archer")
else ()
  add_archer_loadable_module(LLVMArcher
    Archer.cpp
  )
  
  target_link_libraries(LLVMArcher Archer)

  set_target_properties(LLVMArcher
    PROPERTIES
    LINKER_LANGUAGE CXX
    PREFIX "")
endif ()

if (TARGET intrinsics_gen)
  # Check if we are building as part of an LLVM build
  add_dependencies(Archer intrinsics_gen)
endif()
