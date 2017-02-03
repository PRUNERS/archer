include(FindPackageHandleStandardArgs)

# use an explicitly given omp path first
FIND_PATH(OMPT_INCLUDE_PATH ompt.h
            PATHS ${OMP_PREFIX}/include ${LLVM_ROOT}/include ${CMAKE_BINARY_DIR}/projects/openmp/runtime/src ${CMAKE_BINARY_DIR}/include /usr /usr/local}
            PATH_SUFFIXES include NO_DEFAULT_PATH)
# if not-found, try again at cmake locations
FIND_PATH(OMPT_INCLUDE_PATH ompt.h)

find_package_handle_standard_args(OMPT DEFAULT_MSG OMPT_INCLUDE_PATH )
