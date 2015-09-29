/*
 * Copyright (c) 2015, Lawrence Livermore National Security, LLC.
 * 
 * Produced at the Lawrence Livermore National Laboratory
 * 
 * Written by Simone Atzeni (simone@cs.utah.edu), Ganesh Gopalakrishnan,
 * Zvonimir Rakamari\'c Dong H. Ahn, Ignacio Laguna, Martin Schulz, and
 * Gregory L. Lee
 * 
 * LLNL-CODE-676696
 * 
 * All rights reserved.
 * 
 * This file is part of Archer. For details, see
 * https://github.com/soarlab/Archer. Please also read Archer/LICENSE.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the disclaimer (as noted below) in
 * the documentation and/or other materials provided with the
 * distribution.
 * 
 * Neither the name of the LLNS/LLNL nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE
 * LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * ArcherDDA.h
 *
 * Created on: Oct 22, 2014
 *     Author: Simone Atzeni
 *    Contact: simone@cs.utah.edu, atzeni1@llnl.gov
 *
 * ArcherDDA - This is a module pass that applies
 * data dependency analysis with Polly.
 * The ouput of this pass is a file that contains blacklisting
 * information for ThreadSanitizer.
 *
 */

#ifndef ARCHER_DDA_H
#define ARCHER_DDA_H

#include "CommonLLVM.h"
#include "Util.h"

#include "polly/ScopPass.h"
#include "isl/ctx.h"

using namespace llvm;
using namespace polly;

namespace archer {
  
  class ArcherDDA : public polly::ScopPass {

  private:
    Scop *S;
    bool fileInitialized;

    void createDir(std::string dir);
    bool getLOCInfo(Scop &Scop, isl_bool isDependency);
    bool initializeFiles(polly::Scop &Scop);

  public:
    static char ID;
    std::string dir;

  ArcherDDA() : polly::ScopPass(ID) {
      fileInitialized = false;
      dir = ".archer/blacklists";
      createDir(dir);
    }
    
    bool runOnScop(Scop &S) override;
    void printScop(llvm::raw_ostream &OS, Scop &S) const override;
    void getAnalysisUsage(AnalysisUsage &AU) const override;
    void releaseMemory() override {};
  };
}

namespace llvm {
  class PassRegistry;
  void initializeArcherDDAPass(llvm::PassRegistry &);
}

#endif /* ARCHER_DDA_H */
