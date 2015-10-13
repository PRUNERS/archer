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
 * InstructionContext.cpp
 *
 * Created on: Feb 28, 2015
 *     Author: Simone Atzeni
 *    Contact: simone@cs.utah.edu
 *
 *
 */

#include "archer/InstructionContext.h"
#include "archer/LinkAllPasses.h"

using namespace archer;

void InstructionContext::createDir(std::string dir) {
  if(llvm::sys::fs::create_directories(Twine(dir), true)) {
    llvm::errs() << "InstructionContext: Unable to create \"" << dir << "\" directory.\n";
    exit(-1);
  }
}

bool InstructionContext::writeFile(std::string header, std::string filename, std::string ext, std::string &content)
{
  if(!header.empty())
    content = header + "\n" + content;
  
  std::error_code ErrInfo;
  std::string FileName = filename + ext;
  tool_output_file F(FileName.c_str(), ErrInfo, llvm::sys::fs::F_Append);

  if (!ErrInfo) {
    F.os() << content;
    F.os().close();
    if (!F.os().has_error()) {
      F.keep();
    }
  } else {    
    errs() << "error opening file " << FileName << " for writing!\n";
    F.os().clear_error();
    return false;
  }
  
  return true;
}

void InstructionContext::getAnalysisUsage(AnalysisUsage &AU) const
{
  // AU.setPreservesAll();
  // AU.addRequired<CallGraphWrapperPass>();
}

/*
 * Store the result on str and return the filename
 */
std::string InstructionContext::getInfoInstruction(std::string *str, Instruction *I, std::string header)
{
  if (DILocation *Loc = I->getDebugLoc()) {
    unsigned Line = Loc->getLine();
    StringRef File = Loc->getFilename();
    StringRef Dir = Loc->getDirectory();
    
    *str = NumberToString<unsigned>(Line) + "," + I->getParent()->getParent()->getName().str() + "," + File.str() + "," + Dir.str();
    
    // errs() << header << str << "\n";
    
    if(File.str().substr(0,2).compare("./") == 0)
      return Dir.str() + "/" + File.str().substr(2);
    else
      return Dir.str() + "/" + File.str();
  }
 
  // errs() << "No debugging information found. Make sure you are compiling with \"-g\" flag.\n";
  *str = "";
  return "";
  // exit(-1);
}

std::string InstructionContext::getInfoFunction(std::string *str, Function *F, std::string header)
{
  for (auto BB = F->begin(), ii = F->end(); BB != ii; ++BB) {
    for (auto I = BB->begin(), iii = BB->end(); I != iii; ++I) {
      if (DILocation *Loc = I->getDebugLoc()) {
        //unsigned Line = Loc->getLine();
        StringRef File = Loc->getFilename();
        StringRef Dir = Loc->getDirectory();
	
	*str = F->getName().str() + "," + File.str() + "," + Dir.str();
	
	// errs() << header << str << "\n";
	
	if(File.str().substr(0,2).compare("./") == 0)
	  return Dir.str() + "/" + File.str().substr(2);
	else
	  return Dir.str() + "/" + File.str();
      }
    }
  }

  // errs() << "No debugging information found. Make sure you are compiling with \"-g\" flag.\n";
  *str = "";
  return "";
  // exit(-1);
}

bool InstructionContext::runOnModule(Module &M)
{
  std::string content;
  std::string pcontent;
  std::string scontent;
  std::string str;
  std::string filename;
  //std::string ModuleName = M.getModuleIdentifier();  

  // errs() << "Module Name: " << ModuleName << "\n";
  
  // For each Function in the Module
  for (auto F = M.begin(), i = M.end(); F != i; ++F) {
    
    // errs() << "Function Name: " << F->getName() << "\n";

    // Ignore function declaration
    if (F->isDeclaration())
	 continue;
   
    // For Each BasicBlock in the 
    errs() << "Analyzing OpenMP constructs...\n";
    for (auto BB = F->begin(), ii = F->end(); BB != ii; ++BB) {
      for (auto I = BB->begin(), iii = BB->end(); I != iii; ++I) {
	Instruction *Inst = &(*I);
	
	//If the function is an ".omp_outlined."
	if(F->getName().find("omp_outlined") != std::string::npos) {
	  // If the instruction is a Load/Store
	  // add to the list of Parallel Instructions
	  if (isa<LoadInst>(Inst) || isa<StoreInst>(Inst)) {
	    parallel_instructions.insert(Inst);
	    // errs() << "Parallel Load/Store Address: " << Inst << " from Function: " << F->getName() << "\n";
	    // errs() << "START - ###################\n";
	    // getInfoInstruction(Inst, "Parallel Load/StoreSIMONE");
	    // std::string str;
	    // llvm::raw_string_ostream rso(str);
	    // I->print(errs());
	    // errs() << "\nEND - ###################\n";
	  }

	  // If the instruction is a function call
	  // And is not in the list of visited function
	  // Insert Function in Queue of parallel functions
          Function *f = NULL;
	  if(isa<CallInst>(Inst)) {
	    f = cast<CallInst>(Inst)->getCalledFunction();

	    if(f == NULL) {
	      Value* c = cast<CallInst>(Inst)->getCalledValue();

	      // special case that happens when some clang warnings are reported
	      // we can find out the called function exactly
	      if (ConstantExpr* ce = dyn_cast<ConstantExpr>(c)) {
		if (ce->isCast()) {
		  Value* castValue = ce->getOperand(0);
		  f = dyn_cast<Function>(castValue);
		}
	      }
	    }
          } else if(isa<InvokeInst>(Inst)) {
            f = cast<InvokeInst>(Inst)->getCalledFunction();

	    if(f == NULL) {
	      Value* c = cast<InvokeInst>(Inst)->getCalledValue();

	      // special case that happens when some clang warnings are reported
	      // we can find out the called function exactly
	      if (ConstantExpr* ce = dyn_cast<ConstantExpr>(c)) {
		if (ce->isCast()) {
		  Value* castValue = ce->getOperand(0);
		  f = dyn_cast<Function>(castValue);
		}
	      }
	    }
          }

          // Insert function call in queue of parallel functions
          if(f != NULL) {
            errs() << f->getName() << "\n";
            if(visited_functions.find(f) == visited_functions.end()) {
              parallel_functions.push(f);
            }
          }
	} else {
	  if (isa<LoadInst>(Inst) || isa<StoreInst>(Inst)) {
	    // insert instruction in list of sequential instructions
	    // only if it is not already in the list of
	    // parallel instructions
	    // This check here is probably not needed
	    std::set<Instruction *>::iterator it = parallel_instructions.find(Inst);
	    if(it == parallel_instructions.end()) {
	      sequential_instructions.insert(Inst);
	    }
	  }
	}
      }
    }
  }

  errs() << "Analyzing Queue...\n";
  
  // For each function in the Queue of Parallel Functions
  while (!parallel_functions.empty()) {
    errs() << "Function to analyze: " << parallel_functions.size() << "\n";
    Function *F = (Function *) parallel_functions.front();
    visited_functions.insert(F);
    
    // Visit Instructions
    // errs() << "Analyzing function: " << F->getName() << "\n";
    for (auto BB = F->begin(), ii = F->end(); BB != ii; ++BB) {
      for (auto I = BB->begin(), iii = BB->end(); I != iii; ++I) {
	Instruction *Inst = &(*I);
	
	// If the instruction is a Load/Store
	// add to the list of Parallel Instructions
	if (isa<LoadInst>(Inst) || isa<StoreInst>(Inst)) {
	  parallel_instructions.insert(Inst);
	  std::set<Instruction *>::iterator it = sequential_instructions.find(Inst);
	  if(it != sequential_instructions.end()) {
	    sequential_instructions.erase(it);
	  }
	  // errs() << "Parallel Load/Store Address: " << Inst << " from Function: " << F->getName() << "\n";
	}
	
	// If the instruction is a function call
	// And is not in the list of visited function
	// Insert Function in Queue of parallel functions
	if(isa<CallInst>(Inst)) {
	  Function *f = cast<CallInst>(Inst)->getCalledFunction();
	  
	  if(f == NULL) {
	    Value* c = cast<CallInst>(Inst)->getCalledValue();
	    
	    // special case that happens when some clang warnings are reported
	    // we can find out the called function exactly
	    if (ConstantExpr* ce = dyn_cast<ConstantExpr>(c)) {
	      if (ce->isCast()) {
		Value* castValue = ce->getOperand(0);
		f = dyn_cast<Function>(castValue);
	      }
	    }
	  }
	  if(f != NULL) {
	    if(visited_functions.find(f) == visited_functions.end()) {
	      // errs() << "Inserting function call: " << f->getName() << " from function: " << F->getName() << "\n";
	      parallel_functions.push(f);
	    }
	  }
	}
      }
    }

    parallel_functions.pop();
  }

  // Too slow, solved with accumulation variable
  // Remove sequential instructions that are also in the parallel list
  // std::string str1, str2;
  // for(auto Inst1 : parallel_instructions) {
  //   getInfoInstruction(&str1, Inst1, "");
  //   std::vector<std::string> tokens1;
  //   split(&tokens1, (char *) str1.c_str(), ",");
  //   for (std::set<Instruction *>::iterator Inst2 = sequential_instructions.begin(); Inst2 != sequential_instructions.end(); Inst2++) {
  //     getInfoInstruction(&str2, *Inst2, "");
  //     std::vector<std::string> tokens2;
  //     split(&tokens2, (char *) str2.c_str(), ",");
  //     if((tokens1.size() >= 3) && (tokens2.size() >= 3) && tokens1[0].compare(tokens2[0]) == 0  && tokens1[2].compare(tokens2[2]) == 0) {
  // 	sequential_instructions.erase(Inst2);
  //     }
  //   }
  // }
  // Too slow, solved with accumulation variable

  std::string path = dir + "/";
  std::string filepath;

  errs() << "Generating Parallel Instructions file...\n";
  for(auto Inst : parallel_instructions) {
    filename = getInfoInstruction(&str, Inst, "Parallel Instruction: ");
    if(!filename.empty() && !str.empty() && pcontent.find(str) == std::string::npos) {
      content = str + "\n";
      pcontent += content;
      if(StringRef(filename).rsplit('/').second.str().empty()) {
	filepath = path + StringRef(filename).rsplit('/').first.str();
	createDir(path);
      } else {
	filepath = StringRef(filename).rsplit('/').first.str() + "/" + path + StringRef(filename).rsplit('/').second.str();
	createDir(StringRef(filename).rsplit('/').first.str() + "/" + path);
      }
      writeFile("", filepath, PI_LINES, content);
      // # Parallel Instructions
    }
  }

  errs() << "Generating Sequential Instructions file...\n";
  for(auto Inst : sequential_instructions) {
    filename = getInfoInstruction(&str, Inst, "Sequential Instruction: ");
    if(!filename.empty() && !str.empty() && scontent.find(str) == std::string::npos && pcontent.find(str) == std::string::npos) {
      content = str + "\n";
      scontent += content;
      filepath = StringRef(filename).rsplit('/').first.str() + "/" + path + StringRef(filename).rsplit('/').second.str();
      createDir(StringRef(filename).rsplit('/').first.str() + "/" + path);
      writeFile("", filepath, SI_LINES, content);
      // # Sequential Instructions
    }
  }

  errs() << "Generating Parallel Functions file...\n";
  for(auto Func : visited_functions) {
    filename = getInfoFunction(&str, Func, "Parallel Function: ");
    if(!filename.empty() && !str.empty() && content.find(str) == std::string::npos) {
      content = str + "\n";
      filepath = StringRef(filename).rsplit('/').first.str() + "/" + path + StringRef(filename).rsplit('/').second.str();
      createDir(StringRef(filename).rsplit('/').first.str() + "/" + path);
      writeFile("", filepath, PF_LINES, content);
      // # Parallel Functions
    } 
  }

  return true;
}

char InstructionContext::ID = 0;
static RegisterPass<InstructionContext> X("archer-instruction-context", "Archer Instruction Context, returns the list of instructions executed sequentially and the list of instruction executed inside an OpenMP parallel region.", false, false);

namespace archer {
  Pass *createInstructionContextPass() { return new InstructionContext(); }
}

INITIALIZE_PASS_BEGIN(InstructionContext, "archer-instruction-context",
                      "Archer Instruction Context, returns the list of instructions executed sequentially and the list of instruction executed inside an OpenMP parallel region.", false, false);
INITIALIZE_PASS_END(InstructionContext, "archer-instruction-context",
                    "Archer Instruction Context, returns the list of instructions executed sequentially and the list of instruction executed inside an OpenMP parallel region.", false, false)
