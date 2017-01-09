include(FindPackageHandleStandardArgs)

# use an explicitly given ompt path first
FIND_PATH(OMP_INCLUDE_PATH omp.h
            PATHS ${LLVM_ROOT}/include ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/include /usr /usr/local ${CMAKE_BINARY_DIR}/projects/openmp/runtime/src}
            PATH_SUFFIXES include NO_DEFAULT_PATH)
# if not-found, try again at cmake locations
#FIND_PATH(OMP_INCLUDE_PATH omp.h)

find_package_handle_standard_args(OpenMP DEFAULT_MSG OMP_INCLUDE_PATH )
