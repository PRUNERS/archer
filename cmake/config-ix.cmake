include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

check_c_compiler_flag(-Werror OPENMP_HAVE_WERROR_FLAG)

check_cxx_compiler_flag(-std=c++11 OPENMP_HAVE_STD_CPP11_FLAG)

# Returns the host triple.
# Invokes has_ompt.guess

function(has_ompt_support path library symbol var)
  set(has_ompt_guess ${CMAKE_CURRENT_SOURCE_DIR}/cmake/has_ompt.guess)
  execute_process(COMMAND sh ${has_ompt_guess} ${path}/${library} ${symbol}
    RESULT_VARIABLE TT_RV
    OUTPUT_VARIABLE TT_OUT
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if( NOT TT_OUT EQUAL 0 )
    message(FATAL_ERROR "OpenMP library does not have OMPT support. Please recompile your OpenMP library with support for OMPT.")
  endif( NOT TT_OUT EQUAL 0 )
  set( ${var} TRUE PARENT_SCOPE )
endfunction(has_ompt_support var)
