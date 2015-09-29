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
 * InstructionContext.h
 *
 * Created on: Feb 28, 2015
 *     Author: Simone Atzeni
 *    Contact: simone@cs.utah.edu
 *
 *
 */

#ifndef INSTRUCTION_CONTEXT_H
#define INSTRUCTION_CONTEXT_H

#include "llvm/Pass.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ToolOutputFile.h"

#include "archer/CommonLLVM.h"
#include "archer/Util.h"

#include <list>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace llvm;

#define DEBUG_TYPE "archer-memoryusage"

namespace archer {

  class InstructionContext : public ModulePass
  {
  private: 
    /* 
       - Queue of Parallel Functions
       - List of visited functions
       - List of Parallel Instructions
       - List of (Supposed) Sequential Instructions (this list is subject to
       change since if I am visiting a parallel function that calls
       the function that contains these instructions I have to move
       them from the List of Sequential Instruction and to the List
       of parallel instructions)
       - List of Instruction executed both in parallel and sequentially
       (ex.: when a function is called in different places within a
       parallel region and not)
    */

    /**
     * Queue of parallel functions
     */
    std::queue<Function *> parallel_functions;

    /**
     * List of Visited Functions 
     */
    std::set<Function *> visited_functions;

    /**
     * List of Parallel Instructions
     */
    std::set<Instruction *> parallel_instructions;

    /**
     * List of (Supposed) Sequential Instructions
     */
    std::set<Instruction *> sequential_instructions;

    std::string dir;

    void createDir(std::string dir);
    bool writeFile(std::string header, std::string filename, std::string ext, std::string &content);

  public:
    static char ID;
  
  InstructionContext() : ModulePass(ID) {
      dir = ".archer/blacklists";
      createDir(dir);
    }

    bool runOnModule(Module &M) override;
    void getAnalysisUsage(AnalysisUsage &AU) const override;
    std::string getInfoInstruction(std::string *str, Instruction *I, std::string header = "");
    std::string getInfoFunction(std::string *str, Function *F, std::string header = "");  
  };
}

namespace llvm {
  class PassRegistry;
  void initializeInstructionContextPass(llvm::PassRegistry &);
}

#endif /* INSTRUCTION_CONTEXT_H */
