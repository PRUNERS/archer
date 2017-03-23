#
# Copyright (c) 2015-2017, Lawrence Livermore National Security, LLC.
#
# Produced at the Lawrence Livermore National Laboratory
#
# Written by Simone Atzeni (simone@cs.utah.edu), Joachim Protze
# (joachim.protze@tu-dresden.de), Jonas Hahnfeld
# (hahnfeld@itc.rwth-aachen.de), Ganesh Gopalakrishnan, Zvonimir
# Rakamaric, Dong H. Ahn, Gregory L. Lee, Ignacio Laguna, and Martin
# Schulz.
#
# LLNL-CODE-727057
#
# All rights reserved.
#
# This file is part of Archer. For details, see
# https://pruners.github.io/archer. Please also read
# https://github.com/PRUNERS/archer/blob/master/LICENSE.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#    Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the disclaimer below.
#
#    Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the disclaimer (as noted below)
#    in the documentation and/or other materials provided with the
#    distribution.
#
#    Neither the name of the LLNS/LLNL nor the names of its contributors
#    may be used to endorse or promote products derived from this
#    software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE
# LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

cmake_minimum_required(VERSION 3.4.3)

if(POLICY CMP0057)
  cmake_policy(SET CMP0057 NEW)
endif()

# Add cmake directory to search for custom cmake functions
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# TSAN-support with annotated OpenMP Runtime
set(LIBOMP_TSAN_SUPPORT FALSE CACHE BOOL "TSAN-support?")

# Disable Archer static analysis support
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

# These include files are in the cmake/ subdirectory
include(LibarcherUtils)

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

# Look for OpenMP runtime
if(${ARCHER_STANDALONE_BUILD})
  set(OMP_PREFIX /usr/local CACHE PATH "Root of OpenMP runtime install")
  if(NOT EXISTS ${OMP_PREFIX}/include/omp.h)
    message(FATAL_ERROR "OMP_PREFIX (${OMP_PREFIX}) is not a valid OpenMP runtime install")
  endif()
else()
  set(OMP_PREFIX ${LLVM_ROOT} CACHE PATH "Root of OpenMP runtime install")
endif()

find_package(Omp)
include_directories(${OMP_INCLUDE_PATH})
link_directories(${OMP_LIB_PATH})
if(NOT ${LIBOMP_TSAN_SUPPORT})
    find_package(Ompt)
    include_directories(${OMPT_INCLUDE_PATH})
endif()

# Setting directory names
set(ARCHER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(ARCHER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
if(NOT ${LIBOMP_TSAN_SUPPORT})
    set(ARCHER_RUNTIME_PATH ${CMAKE_BINARY_DIR}/rtl)
    set(ARCHER_ARCHER_RUNTIME_SUPPRESSIONS_FILE ${CMAKE_CURRENT_SOURCE_DIR}/rtl/suppressions.txt)
    set(ARCHER_RTL libarcher.so)
endif()

set(ARCHER_LIB_PATH "")
if(${ARCHER_STATIC_ANALYSIS_SUPPORT})
    set(ARCHER_LIB_PATH ${CMAKE_BINARY_DIR}/lib)
    set(ARCHER_LIB LLVMArcher.so)
    set(ARCHER_TOOLS_DIR ${ARCHER_BASE_DIR}/tools)
    add_subdirectory(lib)
endif()

if(NOT ${LIBOMP_TSAN_SUPPORT})
    add_subdirectory(rtl)
endif()
add_subdirectory(test)
add_subdirectory(tools)
