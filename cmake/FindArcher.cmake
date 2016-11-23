include(FindPackageHandleStandardArgs)

set(ARCHER_LIB LLVMArcher.so)
find_path(ARCHER_LIB_PATH
  NAMES LLVMArcher.so
  HINTS ${CMAKE_BINARY_DIR}/lib ${CMAKE_BINARY_DIR}/lib ${LLVM_ROOT}/lib ${CMAKE_INSTALL_PREFIX}/lib
  )
find_path(ARCHER_LIB_PATH LLVMArcher.so)

find_package_handle_standard_args(ARCHER DEFAULT_MSG ARCHER_LIB_PATH)
