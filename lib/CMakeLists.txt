if(${ARCHER_STANDALONE_BUILD})
  set(ARCHER_HEADER_FILES ${PROJECT_SOURCE_DIR}/include)
else()
  set(ARCHER_HEADER_FILES ${PROJECT_SOURCE_DIR}/tools/archer/include)
endif()

include_directories(${ARCHER_HEADER_FILES})

set(ARCHER_SOURCE_FILES
  Archer.cpp
  Support/RegisterPasses.cpp
  Support/Util.cpp
  Transforms/Instrumentation/InstrumentParallel.cpp
  )

add_llvm_loadable_module(LLVMArcher
  ${ARCHER_SOURCE_FILES}
  ${ARCHER_HEADER_FILES}
  )
