cmake_minimum_required(VERSION 3.4.3)

if(POLICY CMP0057)
  cmake_policy(SET CMP0057 NEW)
endif()

# Standalone build or part of LLVM?
set(ARCHER_STANDALONE_BUILD FALSE)
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  project(archer C CXX)
  set(ARCHER_STANDALONE_BUILD TRUE)
endif()

if(${ARCHER_STANDALONE_BUILD})
  # LLVM detection part
  set(LLVM_ROOT "" CACHE PATH "Root of LLVM install.")
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
  find_package(LLVM 3.9 MODULE REQUIRED)

  # A bit of a sanity check:
  if( NOT EXISTS ${LLVM_ROOT}/include/llvm )
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

# Setting directory names
set(LIBARCHER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(LIBARCHER_SRC_DIR ${LIBARCHER_BASE_DIR}/src)
set(LIBARCHER_TOOLS_DIR ${LIBARCHER_BASE_DIR}/tools)
set(LIBARCHER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost 1.41 COMPONENTS REQUIRED)
message(STATUS "Boost Include: ${Boost_INCLUDE_DIR}")
message(STATUS "Boost Libraries: ${Boost_LIBRARY_DIRS}")

add_subdirectory(lib)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test")
  find_package(OmptTsan)
  set(ARCHER_LIB_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)
  set(ARCHER_LIB LLVMArcher.so)
  add_subdirectory(test)
endif()
add_subdirectory(tools)
