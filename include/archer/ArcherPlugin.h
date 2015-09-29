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

//===- ArcherPlugin.h ---------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Description
//
//===----------------------------------------------------------------------===//

#ifndef ARCHER_PLUGIN_H
#define ARCHER_PLUGIN_H

#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/CapturedStmt.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Sema/Sema.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "archer/CommonLLVM.h"
#include "archer/Util.h"

#include <iostream>

using namespace std;
using namespace clang;
using namespace llvm;

namespace archer {

  class ArcherDDAClassVisitor : public RecursiveASTVisitor<ArcherDDAClassVisitor>
  {
  private:
    ASTContext *context;
    SourceManager *sourceMgr;
    std::string pffilename;
    OMPInfo omploc;
    FuncInfo funcloc;

    std::string blfilename;
    std::string ddfilename;
    std::string lsfilename;
    std::string fcfilename;
    DDAInfo ddaloc;
    LSInfo lsloc;
    LSInfo fcloc;
    FCInfo fc;
    BLInfo blacklist;

    void parse(std::string pathname, bool value, std::string path, std::string filename);
    void parse(std::string pathname, std::string path, std::string filename, LSInfo &vec);
    void parseFCInfo(std::string pathname, std::string path, std::string filename);
    
  public:
    ArcherDDAClassVisitor(StringRef file);
    void setContext(ASTContext &context);
    void setSourceManager(SourceManager &sourceMgr);
    bool VisitStmt(clang::Stmt* stmt);
    bool VisitFunctionDecl(FunctionDecl *f);
    bool writeFile();
  };

  class ArcherDDAConsumer : public ASTConsumer
  {
  private:
    CompilerInstance &Instance;
    ArcherDDAClassVisitor *visitor;

  public:
    ArcherDDAConsumer(CompilerInstance &Instance, StringRef File) : Instance(Instance) { visitor = new ArcherDDAClassVisitor(File); }
    virtual void HandleTranslationUnit(ASTContext &context);
  };

  class ArcherDDAAction : public PluginASTAction
  {
protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &Compiler, llvm::StringRef File)
{
  return std::unique_ptr<clang::ASTConsumer>(new ArcherDDAConsumer(Compiler, File));
}

    bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string>& args);
    void PrintHelp(llvm::raw_ostream& ros);
  };

}

#endif // ARCHER_PLUGIN_H
