include(FindPackageHandleStandardArgs)

set(ARCHER_LIB LLVMArcher.so)
find_path(ARCHER_LIB_PATH
  NAMES LLVMArcher.so
  HINTS ${CMAKE_CURRENT_BINARY_DIR}/lib ${LLVM_ROOT}/lib
  )
find_path(ARCHER_LIB_PATH LLVMArcher.so)

find_package_handle_standard_args(ARCHER DEFAULT_MSG ARCHER_LIB_PATH)

