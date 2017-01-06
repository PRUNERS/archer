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
<li><a href="#sec-4-3">4.3. Stand-alone Building</a>
<ul>
<li><a href="#sec-4-3-1">4.3.1. OpenMP Runtime Stand-Alone Building</a></li>
</ul>
</li>
<li><a href="#sec-4-4">4.4. Within Clang/LLVM Building</a></li>
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

<img src="resources/images/archer_logo.png" hspace="5" vspace="5" height="45%" width="45%" alt="ARCHER Logo" title="ARCHER" align="right" />

**ARCHER** is a data race detector for OpenMP programs.


ARCHER combines static and dynamic techniques to identify data races
in large OpenMP applications, leading to low runtime and memory
overheads, while still offering high accuracy and precision. It builds
on open-source tools infrastructure such as LLVM, ThreadSanitizer, and
OMPT to provide portability.

# Prerequisites<a id="sec-3" name="sec-3"></a>

To compile ARCHER you need an host Clang/LLVM version >= 3.9, a
CMake version >= 3.4.3.

Ninja build system is preferred. For more information how to obtain
Ninja visit <https://martine.github.io/ninja>.

ARCHER has been tested with the LLVM OpenMP Runtime version >= 3.9,
and with the LLVM OpenMP Runtime with OMPT support currently under
development at <https://github.com/OpenMPToolsInterface/LLVM-openmp>
(under the branch "align-to-tr").

# Installation<a id="sec-4" name="sec-4"></a>

ARCHER has been developed under LLVM 3.9 (for more information visit
<http://llvm.org>).

## Automatic Building<a id="sec-4-1" name="sec-4-1"></a>

For an automatic building script (recommended) visit the GitHub page
<https://github.com/PRUNER/llvm_archer>.

## Manual Building<a id="sec-4-2" name="sec-4-2"></a>

ARCHER comes as an LLVM tool, it can be compiled both as stand-alone
tool or within the Clang/LLVM infrastructure.

In order to obtain and build ARCHER follow the instructions below for
stand-alone or full Clang/LLVM with ARCHER support building
(instructions are based on bash shell, Clang/LLVM 3.9 version, Ninja
build system, and the LLVM OpenMP Runtime with OMPT support).

Note: Using the LLVM OpenMP Runtime version >= 3.9 may results in some
false positives during the data race detection process.

## Stand-alone Building<a id="sec-4-3" name="sec-4-3"></a>

Create a folder to download and build Clang/LLVM and ARCHER:

export ARCHER<sub>BUILD</sub>=$PWD/ArcherBuild
mkdir $ARCHER<sub>BUILD</sub> && cd $ARCHER<sub>BUILD</sub>

Obtain ARCHER:

    git clone git@github.com:PRUNER/archer.git archer

Let us build ARCHER with the following commands:

    export ARCHER_INSTALL=$HOME/usr           # or any other install path
    cd archer
    mkdir build && cd build
    cmake -G Ninja \
     -D CMAKE_C_COMPILER=clang \
     -D CMAKE_CXX_COMPILER=clang++ \
     -D CMAKE_INSTALL_PREFIX:PATH=${ARCHER_INSTALL} \
     -D LIBOMP_TSAN_SUPPORT=TRUE \
     ..
    ninja -j8 -l8                             # or any number of available cores
    ninja install
    cd ../..

### OpenMP Runtime Stand-Alone Building<a id="sec-4-3-1" name="sec-4-3-1"></a>

Obtain LLVM OpenMP Runtime:

    git clone git@github.com:llvm-mirror/openmp.git openmp

Let us build the OpenMP Runtime with the following command:

    export OPENMP_INSTALL=$HOME/usr           # or any other install path
    cd openmp/runtime
    mkdir build && cd build
    cmake -G Ninja \
     -D CMAKE_C_COMPILER=clang \
     -D CMAKE_CXX_COMPILER=clang++ \
     -D CMAKE_BUILD_TYPE=Release \
     -D CMAKE_INSTALL_PREFIX:PATH=$OPENMP_INSTALL \
     -D LIBOMP_TSAN_SUPPORT=TRUE \
     ..
    ninja -j8 -l8                             # or any number of available cores
    ninja install

or obtain LLVM OpenMP Runtime with OMPT support:

    git clone git@github.com:OpenMPToolsInterface/LLVM-openmp.git openmp

Let us build the OpenMP Runtime with the following command:

    export OPENMP_INSTALL=$HOME/usr           # or any other install path
    cd openmp/runtime
    mkdir build && cd build
    cmake -G Ninja \
     -D CMAKE_C_COMPILER=clang \
     -D CMAKE_CXX_COMPILER=clang++ \
     -D CMAKE_BUILD_TYPE=Release \
     -D CMAKE_INSTALL_PREFIX:PATH=$OPENMP_INSTALL \
     -D LIBOMP_OMPT_SUPPORT=on \
     -D LIBOMP_OMPT_BLAME=on \
     -D LIBOMP_OMPT_TRACE=on \
     ..
    ninja -j8 -l8                             # or any number of available cores
    ninja install

## Within Clang/LLVM Building<a id="sec-4-4" name="sec-4-4"></a>

Create a folder to download and build Clang/LLVM and ARCHER:

export ARCHER<sub>BUILD</sub>=$PWD/ArcherBuild
mkdir $ARCHER<sub>BUILD</sub> && cd $ARCHER<sub>BUILD</sub>

Obtain LLVM:

    git clone git@github.com:llvm-mirror/llvm.git llvm_src
    cd llvm_src
    git checkout release_39

Obtain Clang:

    cd tools
    git clone git@github.com:llvm-mirror/clang.git clang
    cd clang
    git checkout release_39
    cd ..

Obtain the LLVM compiler-rt:

    cd projects
    git clone git@github.com:llvm-mirror/compilter-rt.git compiler-rt
    cd compiler-rt
    git checkout release_39
    cd ../..

Obtain LLVM libc++:

    cd projects
    git clone git@github.com:llvm-mirror/libcxx.git
    cd ..

Obtain LLVM libc++abi:

    cd projects
    git clone git@github.com:llvm-mirror/libcxxabi.git
    cd ..

Obtain LLVM libunwind:

    cd projects
    git clone git@github.com:llvm-mirror/libunwind.git
    cd ..

Obtain ARCHER:

    cd projects
    git clone git@github.com:PRUNER/archer.git archer
    cd ..

Obtain LLVM OpenMP Runtime:

    cd projects
    git clone git@github.com:llvm-mirror/openmp.git openmp
    cd ..

or obtain LLVM OpenMP Runtime with OMPT support:

    cd projects
    git clone git@github.com:OpenMPToolsInterface/LLVM-openmp.git
    cd openmp
    git checkout align-to-tr
    cd ../..

Now that we obtained the source code, the following command
will build LLVM/Clang infrastructure with ARCHER support.

First we boostrap clang:

    cd $ARCHER_BUILD
    mkdir -p llvm_bootstrap
    cd llvm_bootstrap
    CC=$(which gcc) CXX=$(which g++) cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLVM_TOOL_ARCHER_BUILD=OFF -DLLVM_TARGETS_TO_BUILD=Native llvm_src
    ninja -j8 -l8
    cd ..
    export LD_LIBRARY_PATH="${PWD}/llvm_boostrap/lib:${LD_LIBRARY_PATH}"
    export PATH="${PWD}/llvm_bootstrap/bin:${PATH}"

Then, we can actually build LLVM/Clang with ARCHER support:

    export LLVM_INSTALL=$HOME/usr           # or any other install path
    mkdir llvm_build && cd llvm_build
    cmake -G Ninja \
     -D CMAKE_C_COMPILER=clang \
     -D CMAKE_CXX_COMPILER=clang++ \
     -D CMAKE_INSTALL_PREFIX:PATH=${LLVM_INSTALL} \
     -D CLANG_DEFAULT_OPENMP_RUNTIME:STRING=libomp \
     -D LLVM_ENABLE_LIBCXX=ON \
     -D LLVM_ENABLE_LIBCXXABI=ON \
     -D LIBCXXABI_USE_LLVM_UNWINDER=ON \
     -D CLANG_DEFAULT_CXX_STDLIB=libc++ \
     -D LIBOMP_TSAN_SUPPORT=TRUE \
     -D ARCHER_LLVM_TOOLS_DIR=$ARCHER_BUILD/llvm_build/bin \
     -D ARCHER_LLVM_LIT_EXECUTABLE=$ARCHER_BUILD/bin/llvm-lit \
     ../llvm_src
    ninja -j8 -l8                           # or any number of available cores
    ninja install

Once the installation completes, you need to setup your environement
to allow ARCHER to work correctly.

Please set the following path variables:

    export PATH=${LLVM_INSTALL}/bin:\${PATH}"
    export LD_LIBRARY_PATH=${LLVM_INSTALL}/lib:\${LD_LIBRARY_PATH}"

To make the environment permanent add the previous lines or
equivalents to your shell start-up script such as "~/.bashrc".

# Usage<a id="sec-5" name="sec-5"></a>

## How to compile<a id="sec-5-1" name="sec-5-1"></a>

ARCHER provides a command to compile your programs with Clang/LLVM
OpenMP and hide all the mechanisms necessary to detect data races
automatically in your OpenMP programs.

The ARCHER compile command is called *clang-archer*, and this can be
used as a drop-in replacement of your compiler command (e.g., clang,
gcc, etc.).

The following are some of the examples of how one can integrate
*clang-archer* into his/her build system.

### Single source<a id="sec-5-1-1" name="sec-5-1-1"></a>

    clang-archer example.c -o example

### Makefile<a id="sec-5-1-2" name="sec-5-1-2"></a>

In your Makefile, set the following variables:

    CC = clang-archer

### Hybrid MPI-OpenMP programs<a id="sec-5-1-3" name="sec-5-1-3"></a>

In your Makefile, set the following variables:

    CC = mpicc -cc=clang-archer

## Options<a id="sec-5-2" name="sec-5-2"></a>

The command *clang-archer* works as a compiler wrapper, all the
options available for clang are also available for *clang-archer*.

# Sponsor<a id="sec-6" name="sec-6"></a>

<img src="resources/images/uofu_logo.png" hspace="15" vspace="5" height="23%" width="23%" alt="UofU Logo" title="University of Utah" style="float:left" /> <img src="resources/images/llnl_logo.png" hspace="70" vspace="5" height="30%" width="30%" alt="LLNL Logo" title="Lawrence Livermore National Laboratory" style="float:center" /> <img src="resources/images/rwthaachen_logo.png" hspace="15" vspace="5" height="23%" width="23%" alt="RWTH AACHEN Logo" title="RWTH AACHEN University" style="float:left" />
