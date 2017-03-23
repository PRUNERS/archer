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

# void libarcher_say(string message_to_user);
# - prints out message_to_user
macro(libarcher_say message_to_user)
  message(STATUS "ARCHER: ${message_to_user}")
endmacro()

# void libarcher_warning_say(string message_to_user);
# - prints out message_to_user with a warning
macro(libarcher_warning_say message_to_user)
  message(WARNING "ARCHER: ${message_to_user}")
endmacro()

# void libarcher_error_say(string message_to_user);
# - prints out message_to_user with an error and exits cmake
macro(libarcher_error_say message_to_user)
  message(FATAL_ERROR "ARCHER: ${message_to_user}")
endmacro()

# libarcher_append(<flag> <flags_list> [(IF_TRUE | IF_FALSE | IF_TRUE_1_0 ) BOOLEAN])
#
# libarcher_append(<flag> <flags_list>)
#   - unconditionally appends <flag> to the list of definitions
#
# libarcher_append(<flag> <flags_list> <BOOLEAN>)
#   - appends <flag> to the list of definitions if BOOLEAN is true
#
# libarcher_append(<flag> <flags_list> IF_TRUE <BOOLEAN>)
#   - appends <flag> to the list of definitions if BOOLEAN is true
#
# libarcher_append(<flag> <flags_list> IF_FALSE <BOOLEAN>)
#   - appends <flag> to the list of definitions if BOOLEAN is false
#
# libarcher_append(<flag> <flags_list> IF_DEFINED <VARIABLE>)
#   - appends <flag> to the list of definitions if VARIABLE is defined
#
# libarcher_append(<flag> <flags_list> IF_TRUE_1_0 <BOOLEAN>)
#   - appends <flag>=1 to the list of definitions if <BOOLEAN> is true, <flag>=0 otherwise
# e.g., libarcher_append("-D USE_FEATURE" IF_TRUE_1_0 HAVE_FEATURE)
#     appends "-D USE_FEATURE=1" if HAVE_FEATURE is true
#     or "-D USE_FEATURE=0" if HAVE_FEATURE is false
macro(libarcher_append flags flag)
  if(NOT (${ARGC} EQUAL 2 OR ${ARGC} EQUAL 3 OR ${ARGC} EQUAL 4))
    libarcher_error_say("libarcher_append: takes 2, 3, or 4 arguments")
  endif()
  if(${ARGC} EQUAL 2)
    list(APPEND ${flags} "${flag}")
  elseif(${ARGC} EQUAL 3)
    if(${ARGV2})
      list(APPEND ${flags} "${flag}")
    endif()
  else()
    if(${ARGV2} STREQUAL "IF_TRUE")
      if(${ARGV3})
        list(APPEND ${flags} "${flag}")
      endif()
    elseif(${ARGV2} STREQUAL "IF_FALSE")
      if(NOT ${ARGV3})
        list(APPEND ${flags} "${flag}")
      endif()
    elseif(${ARGV2} STREQUAL "IF_DEFINED")
      if(DEFINED ${ARGV3})
        list(APPEND ${flags} "${flag}")
      endif()
    elseif(${ARGV2} STREQUAL "IF_TRUE_1_0")
      if(${ARGV3})
        list(APPEND ${flags} "${flag}=1")
      else()
        list(APPEND ${flags} "${flag}=0")
      endif()
    else()
      libarcher_error_say("libarcher_append: third argument must be one of IF_TRUE, IF_FALSE, IF_DEFINED, IF_TRUE_1_0")
    endif()
  endif()
endmacro()

# void libarcher_get_legal_arch(string* return_arch_string);
# - returns (through return_arch_string) the formal architecture
#   string or warns user of unknown architecture
function(libarcher_get_legal_arch return_arch_string)
  if(${IA32})
    set(${return_arch_string} "IA-32" PARENT_SCOPE)
  elseif(${INTEL64})
    set(${return_arch_string} "Intel(R) 64" PARENT_SCOPE)
  elseif(${MIC})
    set(${return_arch_string} "Intel(R) Many Integrated Core Architecture" PARENT_SCOPE)
  elseif(${ARM})
    set(${return_arch_string} "ARM" PARENT_SCOPE)
  elseif(${PPC64BE})
    set(${return_arch_string} "PPC64BE" PARENT_SCOPE)
  elseif(${PPC64LE})
    set(${return_arch_string} "PPC64LE" PARENT_SCOPE)
  elseif(${AARCH64})
    set(${return_arch_string} "AARCH64" PARENT_SCOPE)
  else()
    set(${return_arch_string} "${libarcher_ARCH}" PARENT_SCOPE)
    libarcher_warning_say("libarcher_get_legal_arch(): Warning: Unknown architecture: Using ${libarcher_ARCH}")
  endif()
endfunction()

# void libarcher_check_variable(string var, ...);
# - runs through all values checking if ${var} == value
# - uppercase and lowercase do not matter
# - if the var is found, then just print it out
# - if the var is not found, then error out
function(libarcher_check_variable var)
  set(valid_flag 0)
  string(TOLOWER "${${var}}" var_lower)
  foreach(value IN LISTS ARGN)
    string(TOLOWER "${value}" value_lower)
    if("${var_lower}" STREQUAL "${value_lower}")
      set(valid_flag 1)
      set(the_value "${value}")
    endif()
  endforeach()
  if(${valid_flag} EQUAL 0)
    libarcher_error_say("libarcher_check_variable(): ${var} = ${${var}} is unknown")
  endif()
endfunction()

# void libarcher_get_build_number(string src_dir, string* return_build_number);
# - grab the eight digit build number (or 00000000) from kmp_version.c
function(libarcher_get_build_number src_dir return_build_number)
  # sets file_lines_list to a list of all lines in kmp_version.c
  file(STRINGS "${src_dir}/src/kmp_version.c" file_lines_list)

  # runs through each line in kmp_version.c
  foreach(line IN LISTS file_lines_list)
    # if the line begins with "#define KMP_VERSION_BUILD" then we take not of the build number
    string(REGEX MATCH "^[ \t]*#define[ \t]+KMP_VERSION_BUILD" valid "${line}")
    if(NOT "${valid}" STREQUAL "") # if we matched "#define KMP_VERSION_BUILD", then grab the build number
      string(REGEX REPLACE "^[ \t]*#define[ \t]+KMP_VERSION_BUILD[ \t]+([0-9]+)" "\\1"
           build_number "${line}"
      )
    endif()
  endforeach()
  set(${return_build_number} "${build_number}" PARENT_SCOPE) # return build number
endfunction()

# void libarcher_get_legal_type(string* return_legal_type);
# - set the legal type name Performance/Profiling/Stub
function(libarcher_get_legal_type return_legal_type)
  if(${NORMAL_LIBRARY})
    set(${return_legal_type} "Performance" PARENT_SCOPE)
  elseif(${PROFILE_LIBRARY})
    set(${return_legal_type} "Profiling" PARENT_SCOPE)
  elseif(${STUBS_LIBRARY})
    set(${return_legal_type} "Stub" PARENT_SCOPE)
  endif()
endfunction()

# void libarcher_add_suffix(string suffix, list<string>* list_of_items);
# - returns list_of_items with suffix appended to all items
# - original list is modified
function(libarcher_add_suffix suffix list_of_items)
  set(local_list "")
  foreach(item IN LISTS "${list_of_items}")
    if(NOT "${item}" STREQUAL "")
      list(APPEND local_list "${item}${suffix}")
    endif()
  endforeach()
  set(${list_of_items} "${local_list}" PARENT_SCOPE)
endfunction()

# void libarcher_list_to_string(list<string> list_of_things, string* return_string);
# - converts a list to a space separated string
function(libarcher_list_to_string list_of_things return_string)
  string(REPLACE ";" " " output_variable "${list_of_things}")
  set(${return_string} "${output_variable}" PARENT_SCOPE)
endfunction()

# void libarcher_string_to_list(string str, list<string>* return_list);
# - converts a string to a semicolon separated list
# - what it really does is just string_replace all running whitespace to a semicolon
# - in cmake, a list is strings separated by semicolons: i.e., list of four items, list = "item1;item2;item3;item4"
function(libarcher_string_to_list str return_list)
  set(outstr)
  string(REGEX REPLACE "[ \t]+" ";" outstr "${str}")
  set(${return_list} "${outstr}" PARENT_SCOPE)
endfunction()
