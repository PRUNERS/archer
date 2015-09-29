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

//===- ArcherPlugin.cpp ---------------------------------------------===//
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

#include "archer/ArcherPlugin.h"
#include "llvm/ADT/StringExtras.h"
#include <system_error>
#include <sys/types.h>
#include <sys/stat.h>
#include "llvm/Support/ToolOutputFile.h"

using namespace archer;

ArcherDDAClassVisitor::ArcherDDAClassVisitor(StringRef File)
{
  std::string path;
  std::string filename;

  // pffilename = "blacklists/" + InFile.str() + DD_LINES;

  std::pair<StringRef, StringRef> pathname = File.rsplit('/');
  
  if(pathname.second.empty()) {
    path = "";
    filename = pathname.first.str();
  } else {
    path = pathname.first.str();
    filename = pathname.second.str();
  }

  // // Read file "ddfilename" and fill "ddaloc" struct
  // parse(ddfilename, false, path, filename);

  omploc.path = path;
  omploc.filename = filename;

  funcloc.path = path;
  funcloc.filename = filename;
}

void ArcherDDAClassVisitor::parse(std::string pathname, bool value, std::string path, std::string filename)
{
  if (pathname.empty()) {
    llvm::errs() << "File is empty or does not exist!\n";
    exit(-1);
  }
  
  ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr = MemoryBuffer::getFile(pathname);
  if (std::error_code EC = FileOrErr.getError()) {
    // llvm::errs() << "File '" << pathname + "' does not exist: " + EC.message() + "\n";
    // llvm::errs() << "Can't open file '" << pathname + "': " + EC.message() + "\n";
    // exit(-1);
  } else {
    int LineNo = 1;
    SmallVector<StringRef, 16> Lines;
    SplitString(FileOrErr.get().get()->getBuffer(), Lines, "\n\r");
    ddaloc.path = path;
    ddaloc.filename = filename;
    for (SmallVectorImpl<StringRef>::iterator I = Lines.begin(), E = Lines.end(); I != E; ++I, ++LineNo) {
      if (I->empty() || I->startswith("#"))
	continue;
      std::pair<StringRef, StringRef> values = I->split(',');
      std::pair<StringRef, StringRef> names = values.second.split(',');
      std::string index = values.first.str();
      std::string File = names.first;
      std::string Dir = names.second;
      // llvm::dbgs() << "Info: " << index <<  " - " << File << " - " << Dir << "\n";
      // llvm::dbgs() << StringToNumber<unsigned>(I->str()) << " - " << ddaloc.line_entries[StringToNumber<unsigned>(I->str())] << "\n";
      if(filename.compare(File) == 0)
	ddaloc.line_entries[StringToNumber<unsigned>(index)] = value;
    }
  }
}

void ArcherDDAClassVisitor::parse(std::string pathname, std::string path, std::string filename, LSInfo &vec)
{
  if (pathname.empty()) {
    llvm::errs() << "File is empty or does not exist!\n";
    exit(-1);
  }
  
  ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr = MemoryBuffer::getFile(pathname);
  if (std::error_code EC = FileOrErr.getError()) {
    llvm::errs() << "File '" << pathname + "' does not exist: " + EC.message() + "\n";
    llvm::errs() << "Can't open file '" << pathname + "': " + EC.message() + "\n";
    exit(-1);
  } else {
    int LineNo = 1;
    SmallVector<StringRef, 16> Lines;
    SplitString(FileOrErr.get().get()->getBuffer(), Lines, "\n\r");
    vec.path = path;
    vec.filename = filename;
    for (SmallVectorImpl<StringRef>::iterator I = Lines.begin(), E = Lines.end(); I != E; ++I, ++LineNo) {
      if (I->empty() || I->startswith("#"))
	continue;
      std::pair<StringRef, StringRef> values = I->split(',');
      std::pair<StringRef, StringRef> names1 = values.second.split(',');
      std::pair<StringRef, StringRef> names2 = names1.second.split(',');
      std::string index = values.first.str();
      std::string Function = names1.first;
      std::string File = names2.first;
      std::string Dir = names2.second;
      // llvm::dbgs() << "Info: " << index <<  " - " << Function << " - " << File << " - " << Dir << "\n";
      vec.line_entries.insert(std::make_pair(StringToNumber<unsigned>(index), Function));
    }
  }
}

void ArcherDDAClassVisitor::parseFCInfo(std::string pathname, std::string path, std::string filename)
{
  if (pathname.empty()) {
    llvm::errs() << "File is empty or does not exist!\n";
    exit(-1);
  }
  
  ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr = MemoryBuffer::getFile(pathname);
  if (std::error_code EC = FileOrErr.getError()) {
    llvm::errs() << "File '" << pathname + "' does not exist: " + EC.message() + "\n";
    llvm::errs() << "Can't open file '" << pathname + "': " + EC.message() + "\n";
    exit(-1);
  } else {
    int LineNo = 1;
    SmallVector<StringRef, 16> Lines;
    SplitString(FileOrErr.get().get()->getBuffer(), Lines, "\n\r");
    fc.path = path;
    fc.filename = filename;
    for (SmallVectorImpl<StringRef>::iterator I = Lines.begin(), E = Lines.end(); I != E; ++I, ++LineNo) {
      if (I->empty() || I->startswith("#"))
	continue;
      std::pair<StringRef, StringRef> values = I->split(',');
      std::pair<StringRef, StringRef> names1 = values.second.split(',');
      std::pair<StringRef, StringRef> names2 = names1.second.split(',');
      std::string index = values.first.str();
      std::string Function = names1.first;
      std::string File = names2.first;
      std::string Dir = names2.second;
      // llvm::dbgs() << "Info: " << index <<  " - " << Function << " - " << File << " - " << Dir << "\n";
      fc.line_entries[StringToNumber<unsigned>(index)] = Function;
    }
  }
}

void ArcherDDAClassVisitor::setContext(ASTContext &context)
{
  this->context = &context;
}

void ArcherDDAClassVisitor::setSourceManager(SourceManager &sourceMgr)
{
  this->sourceMgr = &sourceMgr;
}

bool ArcherDDAClassVisitor::VisitFunctionDecl(FunctionDecl *f) 
{
  // Only function definitions (with bodies), not declarations.
  if (f->hasBody() && f->isThisDeclarationADefinition()) {
    // Function name
    DeclarationName DeclName = f->getNameInfo().getName();
    string FuncName = DeclName.getAsString();

    FuncStmt funcStmt(FuncName, sourceMgr->getSpellingLineNumber(f->getSourceRange().getBegin()), sourceMgr->getSpellingLineNumber(f->getSourceRange().getEnd()));
    funcloc.func_stmt.push_back(funcStmt);

    // errs() << "Function Name: " << FuncName << " - " << sourceMgr->getSpellingLineNumber(f->getSourceRange().getBegin()) << " - " << sourceMgr->getSpellingLineNumber(f->getSourceRange().getEnd()) << "\n";
  }
  
  return true;
}

  bool ArcherDDAClassVisitor::VisitStmt(clang::Stmt* stmt) 
  {
    // llvm::dbgs() << "Visiting Statements...";
    // llvm::dbgs() << stmt->getStmtClassName() << "\n";
    // stmt->dump();
  
    // llvm::dbgs() << stmt->getStmtClass() << " - " <<  Stmt::ForStmtClass << " - " << sourceMgr->getSpellingLineNumber(stmt->getLocStart()) << "\n";
    //llvm::dbgs() << ToString(stmt->getStmtClass()) << " - Line:" << sourceMgr->getSpellingLineNumber(stmt->getLocStart()) << "\n";

    switch (stmt->getStmtClass()) {
      // Getting information about pragmas: pragma line, start, end, file, directory
    case Stmt::OMPParallelDirectiveClass:
    case Stmt::OMPForDirectiveClass:
    case Stmt::OMPCriticalDirectiveClass:
    case Stmt::OMPSingleDirectiveClass:
    case Stmt::OMPAtomicDirectiveClass:
    case Stmt::OMPTaskDirectiveClass:
    case Stmt::OMPParallelForDirectiveClass: {
      //llvm::dbgs() << "OMPConstruct - " << sourceMgr->getFilename(stmt->getLocStart()) << " - LOC:" << sourceMgr->getSpellingLineNumber(stmt->getLocStart()) << "\n";
    
      clang::Stmt::child_iterator child = stmt->child_begin();
    
      Stmt *S = cast<Stmt>(*child);
      CapturedStmt *CS = cast<CapturedStmt>(S);

      // OMPStmt ompStmt;
      OMPStmt ompStmt(sourceMgr->getSpellingLineNumber(stmt->getLocStart()), sourceMgr->getSpellingLineNumber(CS->getLocStart()), sourceMgr->getSpellingLineNumber(CS->getLocEnd()), std::string(stmt->getStmtClassName()));
      omploc.omp_stmt.push_back(ompStmt);
					
      // llvm::dbgs() << "Class[" << ToString(stmt->getStmtClass()) << "] - CapturedStmt[" << CS->capture_size() << "] - " << sourceMgr->getFilename(CS->getLocStart()) << " - LOC_START:" << sourceMgr->getSpellingLineNumber(CS->getLocStart()) << " - LOC_END:" << sourceMgr->getSpellingLineNumber(CS->getLocEnd()) << "\n";
    }
      break;
    default:
      break;
    }     
  
    return(true);
  }

bool ArcherDDAClassVisitor::writeFile() {
  std::string dir;
  std::string fileContent;
  std::string str;
  
  // Loads/Stores list
  if(omploc.path.empty())
    dir = ".archer/blacklists";
  else
    dir = omploc.path + "/.archer/blacklists";
  
  if(llvm::sys::fs::create_directories(Twine(dir), true)) {
    llvm::errs() << "Unable to create \"" << dir << "\" directory.\n";
    return false;
  }
  
  // Function calls list
  fileContent = "";
  
  for (std::vector<OMPStmt>::iterator it = omploc.omp_stmt.begin(); it != omploc.omp_stmt.end(); ++it) {
    str = NumberToString<unsigned>(it->pragma_loc) + "," + NumberToString<unsigned>(it->lb_loc) + "," + NumberToString<unsigned>(it->ub_loc) + "," + it->stmt_class + "\n";
    if(fileContent.find(str) == std::string::npos)
      fileContent += str;
  }

  if(!fileContent.empty()) {
    fileContent = "# OpenMP Pragmas Information\n" + fileContent;
    // fileContent = "# OpenMP Pragmas Information\n" + omploc.filename + "," + omploc.path + "\n" + fileContent;
    
    //llvm::dbgs() << fileContent << "\n";
    
    std::string FileName = dir + "/" + omploc.filename + OI_LINES;
    
    std::error_code ErrInfo;
    tool_output_file F(FileName.c_str(), ErrInfo, llvm::sys::fs::F_Text);

    if (!ErrInfo) {
      F.os() << fileContent;
      F.os().close();
    }
    if (!F.os().has_error()) {
      F.keep();
    } else {    
      errs() << "Error opening file for writing!\n";
      F.os().clear_error();
      return false;
    }
  }
  
  // Function calls list
  fileContent = "";
  
  for (std::vector<FuncStmt>::iterator it = funcloc.func_stmt.begin(); it != funcloc.func_stmt.end(); ++it) {
    str = it->func_name + "," + NumberToString<unsigned>(it->lb_loc) + "," + NumberToString<unsigned>(it->ub_loc) + "\n";
    if(fileContent.find(str) == std::string::npos)
      fileContent += str;
  }
  
  if(!fileContent.empty()) {
    fileContent = "# Function Information\n" + funcloc.filename + "," + funcloc.path + "\n" + fileContent;
    
    //llvm::dbgs() << fileContent << "\n";
    
    std::string FileName = dir + "/" + funcloc.filename + FI_LINES;
    
    std::error_code ErrInfo;
    tool_output_file F(FileName.c_str(), ErrInfo, llvm::sys::fs::F_Text);
    
    if (ErrInfo) {
      F.os() << fileContent;
      F.os().close();
    }
    if (!F.os().has_error()) {
      F.keep();
    } else {    
      errs() << "Error opening file for writing!\n";
      F.os().clear_error();
      return false;
    }
  }
  
  return true; 
}

void ArcherDDAConsumer::HandleTranslationUnit(ASTContext &context) 
{
  visitor->setContext(context);
  visitor->setSourceManager(context.getSourceManager());
  visitor->TraverseDecl(context.getTranslationUnitDecl());
  visitor->writeFile();
}

bool ArcherDDAAction::ParseArgs(const CompilerInstance &CI, const std::vector<std::string>& args) 
{
  for (unsigned i = 0, e = args.size(); i != e; ++i) {
    llvm::errs() << "ArcherDDAParser arg = " << args[i] << "\n";
    
    // Example error handling.
    if (args[i] == "-an-error") {
      DiagnosticsEngine &D = CI.getDiagnostics();
      unsigned DiagID = D.getCustomDiagID(DiagnosticsEngine::Error, "invalid argument '%0'");
      D.Report(DiagID) << args[i];
      return false;
    }
  }
  if (args.size() && args[0] == "help")
    PrintHelp(llvm::errs());
  
  return true;
}

void ArcherDDAAction::PrintHelp(llvm::raw_ostream& ros)
{
  ros << "Help for ArcherDDAParser plugin goes here\n";
}

static FrontendPluginRegistry::Add<ArcherDDAAction>
X("archer", "Generate blacklisting information exploiting results from different analysis (Polly Data Dependency Analysis, etc.)");
