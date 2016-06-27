<div id="table-of-contents">
<h2>Table of Contents</h2>
<div id="text-table-of-contents">
<ul>
<li><a href="#sec-1">1. License</a></li>
<li><a href="#sec-2">2. Introduction</a></li>
<li><a href="#sec-3">3. Prerequisites</a></li>
<li><a href="#sec-4">4. Installation</a>
<ul>
<li><a href="#sec-4-1">4.1. Automatic Building</a></li>
<li><a href="#sec-4-2">4.2. Manual Building</a></li>
</ul>
</li>
<li><a href="#sec-5">5. Usage</a>
<ul>
<li><a href="#sec-5-1">5.1. How to compile</a>
<ul>
<li><a href="#sec-5-1-1">5.1.1. Single source</a></li>
<li><a href="#sec-5-1-2">5.1.2. Makefile</a></li>
<li><a href="#sec-5-1-3">5.1.3. Hybrid MPI-OpenMP programs</a></li>
</ul>
</li>
<li><a href="#sec-5-2">5.2. Options</a></li>
</ul>
</li>
<li><a href="#sec-6">6. Sponsor</a></li>
</ul>
</div>
</div>


# License<a id="sec-1" name="sec-1"></a>

Please see LICENSE for usage terms.

# Introduction<a id="sec-2" name="sec-2"></a>

<img src="resources/images/archer_logo.png" hspace="5" vspace="5" height="45%" width="45%" alt="Archer Logo" title="Archer" align="right" />

**Archer** is a data race detector for OpenMP programs.


Archer combines static and dynamic techniques to identify data races
in large OpenMP applications, leading to low runtime and memory
overheads, while still offering high accuracy and precision. It builds
on open-source tools infrastructure such as LLVM and ThreadSanitizer
to provide portability.

# Prerequisites<a id="sec-3" name="sec-3"></a>

To compile Archer you need an host GCC version >= 4.7 and a CMake
version >= 3.0.

Ninja build system is preferred. For more information how to obtain
Ninja visit <https://martine.github.io/ninja/>.

# Installation<a id="sec-4" name="sec-4"></a>

Archer has been developed under LLVM 3.8 (for
more information visit <http://llvm.org>).

## Automatic Building<a id="sec-4-1" name="sec-4-1"></a>

For an automatic building (recommended) script visit the GitHub page
<https://github.com/PRUNER/llvm_archer>.

## Manual Building<a id="sec-4-2" name="sec-4-2"></a>

Archer comes as an LLVM tool, in order to compile it we must compile
the entire LLVM/Clang infrastructure. Archer developers provide a
patched LLVM/Clang version based on the main LLVM/Clang trunk.

In order to obtain and build LLVM/Clang with Archer execute the
following commands in your command-line (instructions are based on
bash shell, GCC-4.9.3 version and Ninja build system).

    export ARCHER_BUILD=$PWD/ArcherBuild
    mkdir $ARCHER_BUILD && cd $ARCHER_BUILD

Obtain the Archer patched LLVM version:

    git clone git@github.com:PRUNER/llvm.git llvm_src
    cd llvm_src
    git checkout archer
    git checkout tags/1.0.0

Obtain the Archer patched Clang version:

    cd tools
    git clone git@github.com:PRUNER/clang.git clang
    cd clang
    git checkout archer
    git checkout tags/1.0.0
    cd ..

Obtain Polly:

    git clone git@github.com:llvm-mirror/polly.git polly
    cd polly
    git checkout tags/1.0.0
    cd ..

Obtain Archer:

    git clone git@github.com:PRUNER/archer.git archer
    cd archer
    git checkout tags/1.0.0
    cd ../..

Obtain the LLVM runtime:

    cd projects
    git clone git@github.com:PRUNER/compilter-rt.git compiler-rt
    cd compiler-rt
    git checkout tags/1.0.0
    cd ..

Obtain LLVM OpenMP Runtime with support for Archer:

    git clone git@github.com:PRUNER/openmp.git openmp
    cd openmp
    git checkout annotations
    git checkout tags/1.0.0

Obtain LLVM libc++:

    git clone git@github.com:llvm-mirror/libcxx.git

Obtain LLVM libc++abi:

    git clone git@github.com:llvm-mirror/libcxxabi.git

Obtain LLVM libunwind:

    git clone git@github.com:llvm-mirror/libunwind.git

Now that we obtained the source code, the following command
will build the patched LLVM/Clang version with Archer support.

First we boostrap clang:

    cd $ARCHER_BUILD
    mkdir -p llvm_bootstrap
    cd llvm_bootstrap
    CC=$(which gcc) CXX=$(which g++) cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLVM_TOOL_ARCHER_BUILD=OFF -DLLVM_TARGETS_TO_BUILD=Native llvm_src
    ninja -j8 -l8
    cd ..
    export LD_LIBRARY_PATH="${PWD}/llvm_boostrap/lib:${LD_LIBRARY_PATH}"
    export PATH="${PWD}/llvm_bootstrap/bin:${PATH}"

Then, we can actually build LLVM/Clang:

    export BOOST_ROOT=/path/to/boost
    export LLVM_INSTALL=$HOME/usr           # or any other install path
    mkdir llvm_build && cd llvm_build
    cmake -G Ninja \
     -D CMAKE_C_COMPILER=clang \
     -D CMAKE_CXX_COMPILER=clang++ \
     -D CMAKE_INSTALL_PREFIX:PATH=${LLVM_INSTALL} \
     -D LINK_POLLY_INTO_TOOLS:Bool=ON \
     -D CLANG_DEFAULT_OPENMP_RUNTIME:STRING=libomp \
     -D LIBOMP_TSAN_SUPPORT=TRUE \
     -D CMAKE_BUILD_TYPE=Ninja \
     -D LLVM_ENABLE_LIBCXX=ON \
     -D LLVM_ENABLE_LIBCXXABI=ON \
     -D LIBCXXABI_USE_LLVM_UNWINDER=ON \
     -D CLANG_DEFAULT_CXX_STDLIB=libc++ \
     -DBOOST_ROOT=${BOOST_ROOT} \
     -DBOOST_LIBRARYDIR=${BOOST_ROOT}/lib \
     -DBoost_NO_SYSTEM_PATHS=ON \
     ../llvm_src
    ninja -j8 -l8
    ninja install


Once the installation completes, you need to setup your environement
to allow Archer to work correctly.

Please set the following path variables:

    export PATH=${LLVM_INSTALL}/bin:${LLVM_INSTALL}/bin/archer:\${PATH}"
    export LD_LIBRARY_PATH=${LLVM_INSTALL}/lib:\${LD_LIBRARY_PATH}"

To make the environment permanent add the previous lines or
equivalents to your shell start-up script such as "~/.bashrc".

# Usage<a id="sec-5" name="sec-5"></a>

## How to compile<a id="sec-5-1" name="sec-5-1"></a>

Archer provides a command to compile your programs with Clang/LLVM
OpenMP and hide all the mechanics necessary to detect data races
automatically in your OpenMP programs.

This Archer command is called *clang-archer*, and this can be used as
a drop-in replacement of your compiler command (e.g., clang, gcc,
etc.).

The following are some of the examples of how one can integrate
*clang-archer* into his/her build system.

### Single source<a id="sec-5-1-1" name="sec-5-1-1"></a>

    clang-archer example.c -L/path/to/openmp/runtime -lOMPRT -o example

### Makefile<a id="sec-5-1-2" name="sec-5-1-2"></a>

In your Makefile, set the following variables:

    CC = clang-archer
    
    LDFLAGS = -L/path/to/openmp/runtime -lOMPRT

### Hybrid MPI-OpenMP programs<a id="sec-5-1-3" name="sec-5-1-3"></a>

In your Makefile, set the following variables:

    CC = mpicc -cc=clang-archer
    
    ...
    
    LDFLAGS = -L/path/to/openmp/runtime -lOMPRT

## Options<a id="sec-5-2" name="sec-5-2"></a>

Running the following command:

    clang-archer --help

shows the options available with *clang-archer*.

    usage: clang-archer [-h] [-v] [-d] [--log] [-db] [-CC [CC]] [-USE_MPI]
                        [-MPICC [MPICC]] [-OPT [OPT]] [-LINK [LINK]] [-DIS [DIS]]
                        [-LIB [LIB]] [-PLUGIN_LIB [PLUGIN_LIB]]
                        [-OPENMP_INCLUDE [OPENMP_INCLUDE]] [-g]
                        [-O0 | -O1 | -O2 | -O3 | -Os | -Oz] [-fopenmp] [-liomp5]
                        [-c] [-o [O]]
    
    Compile your program with Archer support, a data race detector for OpenMP programs.
    
    optional arguments:
      -h, --help            show this help message and exit
      -v, --version         show program's version number and exit
      -d, --debug           Print the compiling commands
      --log                 Keep intermediate logs
      -db, --disable-blacklisting
                            Disable static analysis and apply ThreadSanitizer
                            instrumentation to the entire program
      -CC [CC]              Change the program used to compile and link the
                            programs
      -USE_MPI              Link against MPI libraries
      -MPICC [MPICC]        Change the program used to compile and link the MPI
                            programs
      -OPT [OPT]            Change the program used to optmize the programs
      -LINK [LINK]          Change the program used to link the byte code files
      -DIS [DIS]            Change the program used to disassemble the byte code
                            files
      -LIB [LIB]            Set the path where to find Archer libraries
      -PLUGIN_LIB [PLUGIN_LIB]
                            Set the path where to find Archer Plugin libraries
      -OPENMP_INCLUDE [OPENMP_INCLUDE]
                            Set the path where to find OpenMP headers
      -g                    If the debugging flag is not present in the
                            compilation command it will be added by default
      -O0                   The optimization flags will be forced to '-O0'
                            optimization level for analysis purposes
      -O1
      -O2
      -O3
      -Os
      -Oz
      -fopenmp              OpenMP flag
      -liomp5               OpenMP library
      -c                    Only run preprocess, compile, and assemble steps
      -o [O]                Output filename

# Sponsor<a id="sec-6" name="sec-6"></a>

<img src="resources/images/uofu_logo.png" hspace="5" vspace="5" height="35%" width="35%" alt="UofU Logo" title="University of Utah" align="left" />

<img src="resources/images/llnl_logo.png" hspace="5" vspace="5" height="50%" width="50%" alt="LLNL Logo" title="Lawrence Livermore National Laboratory" align="right" />