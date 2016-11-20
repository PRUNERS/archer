if(${ARCHER_STANDALONE_BUILD})
  set(ARCHER_HEADER_FILES ${PROJECT_SOURCE_DIR}/include)
else()
  set(ARCHER_HEADER_FILES ${PROJECT_SOURCE_DIR}/tools/archer/include)
endif()

include_directories(${ARCHER_HEADER_FILES})

add_llvm_loadable_module(LLVMArcher
  Archer.cpp
  Support/RegisterPasses.cpp
  Support/Util.cpp
  Transforms/Instrumentation/InstrumentParallel.cpp
  ${ARCHER_HEADER_FILES}
  )
