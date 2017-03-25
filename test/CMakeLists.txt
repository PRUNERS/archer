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

include(FindPythonInterp)
include(CheckTypeSize)
include(CheckLibraryExists)

if(NOT PYTHONINTERP_FOUND)
  libarcher_warning_say("Could not find Python.")
  libarcher_warning_say("The check-archer target will not be available!")
  return()
endif()

# Some tests use math functions
check_library_exists(m sqrt "" ARCHER_HAVE_LIBM)
set(ARCHER_HAVE_ARCHER_LIBRARY ${ARCHER_LIB_PATH})
set(ARCHER_HAVE_ARCHER_RUNTIME ${ARCHER_RUNTIME_PATH})

macro(pythonize_bool var)
  if (${var})
    set(${var} True)
  else()
    set(${var} False)
  endif()
endmacro()

pythonize_bool(ARCHER_HAVE_LIBM)
pythonize_bool(ARCHER_HAVE_ARCHER_LIBRARY)
pythonize_bool(ARCHER_HAVE_ARCHER_RUNTIME)

set(ARCHER_TEST_CFLAGS "" CACHE STRING
  "Extra compiler flags to send to the test compiler")

if(${ARCHER_STANDALONE_BUILD})
  # Make sure we can use the console pool for recent cmake and ninja > 1.5
  if(CMAKE_VERSION VERSION_LESS 3.1.20141117)
    set(cmake_3_2_USES_TERMINAL)
  else()
    set(cmake_3_2_USES_TERMINAL USES_TERMINAL)
  endif()
  set(ARCHER_TEST_COMPILER ${CMAKE_C_COMPILER} CACHE STRING
    "Compiler to use for testing Archer library")
  set(ARCHER_TEST_OPENMP_FLAG -fopenmp CACHE STRING
    "Compiler flag to use for testing Archer library")
  find_program(ARCHER_LLVM_LIT_EXECUTABLE
    NAMES llvm-lit lit.py lit
    PATHS ${ARCHER_LLVM_TOOLS_DIR})
  if(NOT ARCHER_LLVM_LIT_EXECUTABLE)
    libarcher_say("Cannot find llvm-lit.")
    libarcher_say("Please put llvm-lit in your PATH, set ARCHER_LLVM_LIT_EXECUTABLE to its full path or point ARCHER_LLVM_TOOLS_DIR to its directory")
    libarcher_warning_say("The check-libarcher target will not be available!")
    return()
  endif()
  find_program(FILECHECK_EXECUTABLE
    NAMES FileCheck
    PATHS ${ARCHER_LLVM_TOOLS_DIR})
  if (NOT FILECHECK_EXECUTABLE)
    # set to empty string so that lit can disable dependent tests
    set(FILECHECK_EXECUTABLE "")
  endif()
  # Set lit arguments
  # The -j 1 lets the actual tests run with the entire machine.
  # We have one test thread that spawns the tests serially.  This allows
  # Each test to use the entire machine.
  set(ARCHER_LIT_ARGS_DEFAULT "-sv --show-unsupported --show-xfail")
  if(MSVC OR XCODE)
    set(ARCHER_LIT_ARGS_DEFAULT "${ARCHER_LIT_ARGS_DEFAULT} --no-progress-bar")
  endif()
  set(ARCHER_LIT_ARGS "${ARCHER_LIT_ARGS_DEFAULT}" CACHE STRING
    "Default options for lit")
  separate_arguments(ARCHER_LIT_ARGS)
  add_custom_target(check-libarcher
    COMMAND ${PYTHON_EXECUTABLE} ${ARCHER_LLVM_LIT_EXECUTABLE} ${ARCHER_LIT_ARGS} ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running libarcher tests"
    ${cmake_3_2_USES_TERMINAL}
  )
else()
  # LLVM source tree build, test just-built clang
  if(NOT MSVC)
    set(ARCHER_TEST_COMPILER ${LLVM_RUNTIME_OUTPUT_INTDIR}/clang)
  else()
    set(ARCHER_TEST_COMPILER ${LLVM_RUNTIME_OUTPUT_INTDIR}/clang.exe)
  endif()
  set(FILECHECK_EXECUTABLE ${LLVM_RUNTIME_OUTPUT_INTDIR}/FileCheck)
  # Use add_lit_testsuite() from LLVM CMake.
  add_lit_testsuite(check-libarcher
    "Running libarcher tests"
    ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS clang clang-headers FileCheck omp
  )
endif()

# Configure the lit.site.cfg.in file
set(AUTO_GEN_COMMENT "## Autogenerated by libarcher configuration.\n# Do not edit!")
configure_file(lit.site.cfg.in lit.site.cfg @ONLY)
