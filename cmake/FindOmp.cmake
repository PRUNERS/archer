include(FindPackageHandleStandardArgs)

# use an explicitly given omp path first
FIND_PATH(OMP_INCLUDE_PATH omp.h
            PATHS ${OMP_PREFIX}/include ${LLVM_ROOT}/include ${CMAKE_BINARY_DIR}/projects/openmp/runtime/src ${CMAKE_BINARY_DIR}/include /usr /usr/local}
            PATH_SUFFIXES include NO_DEFAULT_PATH)
# if not-found, try again at cmake locations
FIND_PATH(OMP_INCLUDE_PATH omp.h)

find_package_handle_standard_args(OpenMP DEFAULT_MSG OMP_INCLUDE_PATH )

# use an explicitly given omp path first
FIND_PATH(OMP_LIB_PATH libomp.so
            PATHS ${OMP_PREFIX}/lib ${LLVM_ROOT}/lib ${CMAKE_INSTALL_PREFIX}/lib /usr /usr/local}
            PATH_SUFFIXES lib NO_DEFAULT_PATH)
# if not-found, try again at cmake locations
FIND_PATH(OMP_LIB_PATH libomp.so)

find_package_handle_standard_args(OpenMP DEFAULT_MSG OMP_LIB_PATH )
