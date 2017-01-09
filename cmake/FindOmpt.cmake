include(FindPackageHandleStandardArgs)

# use an explicitly given ompt path first
FIND_PATH(OMPT_INCLUDE_PATH ompt.h
            PATHS ${LLVM_ROOT}/include ${CMAKE_INSTALL_PREFIX}/include /usr /usr/local ${CMAKE_INSTALL_PREFIX}/projects/openmp/runtime/src}
            PATH_SUFFIXES include NO_DEFAULT_PATH)
# if not-found, try again at cmake locations
FIND_PATH(OMPT_INCLUDE_PATH ompt.h)

find_package_handle_standard_args(OMPT DEFAULT_MSG OMPT_INCLUDE_PATH )
