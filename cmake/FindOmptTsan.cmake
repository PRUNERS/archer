include(FindPackageHandleStandardArgs)

set(OMPT_TSAN_LIB ompt-tsan.so)
find_path(OMPT_TSAN_LIB_PATH
  NAMES ompt-tsan.so
  HINTS ${CMAKE_BINARY_DIR}/src ${CMAKE_BINARY_DIR}/lib ${LLVM_ROOT}/lib ${CMAKE_INSTALL_PREFIX}/lib
)
find_path(OMPT_TSAN_LIB_PATH ompt-tsan.so)

find_package_handle_standard_args(OMPT_TSAN DEFAULT_MSG OMPT_TSAN_LIB_PATH)
