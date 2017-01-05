cmake_minimum_required(VERSION 3.4.3)

if(POLICY CMP0057)
  cmake_policy(SET CMP0057 NEW)
endif()

# TSAN-support with annotated OpenMP Runtime
set(LIBOMP_ANNOTATION_TSAN_SUPPORT FALSE CACHE BOOL "TSAN-support?")
set(ARCHER_STATIC_ANALYSIS_SUPPORT TRUE CACHE BOOL "StaticAnalysisSupport?")

# Standalone build or part of LLVM?
set(ARCHER_STANDALONE_BUILD FALSE)
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    project(archer C CXX)

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        # require at least clang 3.9
        if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.9)
            message(FATAL_ERROR "Clang version must be at least 3.9!")
        endif()
    else()
        message(FATAL_ERROR "You are using an unsupported compiler! The required compiler is Clang version >= 3.9.")
    endif()
    
    if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
        # require at least clang 3.9
        if (CMAKE_C_COMPILER_VERSION VERSION_LESS 3.9)
            message(FATAL_ERROR "Clang version must be at least 3.9!")
        endif()
    else()
        message(FATAL_ERROR "You are using an unsupported compiler! The required compiler is Clang version >= 3.9.")
    endif()
  set(ARCHER_STANDALONE_BUILD TRUE)
  string(SUBSTRING ${CMAKE_CXX_COMPILER_VERSION} 0 3 LLVM_VERSION)
  string(REPLACE "." "" LLVM_VERSION ${LLVM_VERSION})
  add_definitions(-DLLVM_VERSION=${LLVM_VERSION})
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(${ARCHER_STANDALONE_BUILD})
  # LLVM detection part
  set(LLVM_ROOT "" CACHE PATH "Root of LLVM install.")
  find_package(LLVM MODULE REQUIRED)

  # Sanity check
  if(NOT EXISTS ${LLVM_ROOT}/include/llvm)
    message(FATAL_ERROR "LLVM_ROOT (${LLVM_ROOT}) is not a valid LLVM install")
  endif()

  # Incorporate the CMake features provided by LLVM:
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${LLVM_ROOT}/lib/cmake/llvm")

  include(LLVMConfig)
  include(HandleLLVMOptions)
  include(AddLLVM)

  add_definitions(${LLVM_DEFINITIONS})
  include_directories(${LLVM_INCLUDE_DIRS})
endif()

find_package(Omp)
find_package(Ompt)

include_directories(${OMP_INCLUDE_PATH})
include_directories(${OMPT_INCLUDE_PATH})

# Setting directory names
set(ARCHER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(ARCHER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(ARCHER_RUNTIME_PATH ${CMAKE_BINARY_DIR}/rtl)
set(ARCHER_ARCHER_RUNTIME_SUPPRESSIONS_FILE ${CMAKE_CURRENT_SOURCE_DIR}/rtl/suppressions.txt)
set(ARCHER_RTL libarcher.so)

set(ARCHER_LIB_PATH "")
if(${ARCHER_STATIC_ANALYSIS_SUPPORT})
    set(ARCHER_LIB_PATH ${CMAKE_BINARY_DIR}/lib)
    set(ARCHER_LIB LLVMArcher.so)
    set(ARCHER_TOOLS_DIR ${ARCHER_BASE_DIR}/tools)
    add_subdirectory(lib)
endif()

add_subdirectory(rtl)
add_subdirectory(test)
add_subdirectory(tools)
