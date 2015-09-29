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

//===- Common.h ---------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Common definition and data structures for Archer.
//
//===----------------------------------------------------------------------===//

#ifndef COMMON_LLVM_H
#define COMMON_LLVM_H

#include "clang/AST/Stmt.h"
#include "archer/Common.h"

inline const char* ToString(clang::Stmt::StmtClass v) {
  switch (v) {
  case clang::Stmt::NullStmtClass: return "NullStmt";
  case clang::Stmt::CompoundStmtClass: return "CompoundStmt";
  case clang::Stmt::LabelStmtClass: return "LabelStmt";
  case clang::Stmt::AttributedStmtClass: return "AttributedStmt";
  case clang::Stmt::IfStmtClass: return "IfStmt";
  case clang::Stmt::SwitchStmtClass: return "SwitchStmt";
  case clang::Stmt::WhileStmtClass: return "WhileStmt";
  case clang::Stmt::DoStmtClass: return "DoStmt";
  case clang::Stmt::ForStmtClass: return "ForStmt";
  case clang::Stmt::GotoStmtClass: return "GotoStmt";
  case clang::Stmt::IndirectGotoStmtClass: return "IndirectGotoStmt";
  case clang::Stmt::ContinueStmtClass: return "ContinueStmt";
  case clang::Stmt::BreakStmtClass: return "BreakStmt";
  case clang::Stmt::ReturnStmtClass: return "ReturnStmt";
  case clang::Stmt::DeclStmtClass: return "DeclStmt";
  /* case clang::Stmt::SwitchCaseClass: return "SwitchCase"; */
  case clang::Stmt::CaseStmtClass: return "CaseStmt";
  case clang::Stmt::DefaultStmtClass: return "DefaultStmt";
  case clang::Stmt::CapturedStmtClass: return "CapturedStmt";
  /* case clang::Stmt::AsmStmtClass: return "AsmStmt"; */
  case clang::Stmt::GCCAsmStmtClass: return "GCCAsmStmt";
  case clang::Stmt::MSAsmStmtClass: return "MSAsmStmt";
  case clang::Stmt::ObjCAtTryStmtClass: return "ObjCAtTryStmt";
  case clang::Stmt::ObjCAtCatchStmtClass: return "ObjCAtCatchStmt";
  case clang::Stmt::ObjCAtFinallyStmtClass: return "ObjCAtFinallyStmt";
  case clang::Stmt::ObjCAtThrowStmtClass: return "ObjCAtThrowStmt";
  case clang::Stmt::ObjCAtSynchronizedStmtClass: return "ObjCAtSynchronizedStmt";
  case clang::Stmt::ObjCForCollectionStmtClass: return "ObjCForCollectionStmt";
  case clang::Stmt::ObjCAutoreleasePoolStmtClass: return "ObjCAutoreleasePoolStmt";
  case clang::Stmt::CXXCatchStmtClass: return "CXXCatchStmt";
  case clang::Stmt::CXXTryStmtClass: return "CXXTryStmt";
  case clang::Stmt::CXXForRangeStmtClass: return "CXXForRangeStmt";
  /* case clang::Stmt::ExprClass: return "Expr"; */
  case clang::Stmt::PredefinedExprClass: return "PredefinedExpr";
  case clang::Stmt::DeclRefExprClass: return "DeclRefExpr";
  case clang::Stmt::IntegerLiteralClass: return "IntegerLiteral";
  case clang::Stmt::FloatingLiteralClass: return "FloatingLiteral";
  case clang::Stmt::ImaginaryLiteralClass: return "ImaginaryLiteral";
  case clang::Stmt::StringLiteralClass: return "StringLiteral";
  case clang::Stmt::CharacterLiteralClass: return "CharacterLiteral";
  case clang::Stmt::ParenExprClass: return "ParenExpr";
  case clang::Stmt::UnaryOperatorClass: return "UnaryOperator";
  case clang::Stmt::OffsetOfExprClass: return "OffsetOfExpr";
  case clang::Stmt::UnaryExprOrTypeTraitExprClass: return "UnaryExprOrTypeTraitExpr";
  case clang::Stmt::ArraySubscriptExprClass: return "ArraySubscriptExpr";
  case clang::Stmt::CallExprClass: return "CallExpr";
  case clang::Stmt::MemberExprClass: return "MemberExpr";
  /* case clang::Stmt::CastExprClass: return "CastExpr"; */
  case clang::Stmt::BinaryOperatorClass: return "BinaryOperator";
  case clang::Stmt::CompoundAssignOperatorClass: return "CompoundAssignOperator";
  /* case clang::Stmt::AbstractConditionalOperatorClass: return "AbstractConditionalOperator"; */
  case clang::Stmt::ConditionalOperatorClass: return "ConditionalOperator";
  case clang::Stmt::BinaryConditionalOperatorClass: return "BinaryConditionalOperator";
  case clang::Stmt::ImplicitCastExprClass: return "ImplicitCastExpr";
  /* case clang::Stmt::ExplicitCastExprClass: return "ExplicitCastExpr"; */
  case clang::Stmt::CStyleCastExprClass: return "CStyleCastExpr";
  case clang::Stmt::CompoundLiteralExprClass: return "CompoundLiteralExpr";
  case clang::Stmt::ExtVectorElementExprClass: return "ExtVectorElementExpr";
  case clang::Stmt::InitListExprClass: return "InitListExpr";
  case clang::Stmt::DesignatedInitExprClass: return "DesignatedInitExpr";
  case clang::Stmt::ImplicitValueInitExprClass: return "ImplicitValueInitExpr";
  case clang::Stmt::ParenListExprClass: return "ParenListExpr";
  case clang::Stmt::VAArgExprClass: return "VAArgExpr";
  case clang::Stmt::GenericSelectionExprClass: return "GenericSelectionExpr";
  case clang::Stmt::PseudoObjectExprClass: return "PseudoObjectExpr";
  case clang::Stmt::AtomicExprClass: return "AtomicExpr";
  case clang::Stmt::AddrLabelExprClass: return "AddrLabelExpr";
  case clang::Stmt::StmtExprClass: return "StmtExpr";
  case clang::Stmt::ChooseExprClass: return "ChooseExpr";
  case clang::Stmt::GNUNullExprClass: return "GNUNullExpr";
  case clang::Stmt::CXXOperatorCallExprClass: return "CXXOperatorCallExpr";
  case clang::Stmt::CXXMemberCallExprClass: return "CXXMemberCallExpr";
  /* case clang::Stmt::CXXNamedCastExprClass: return "CXXNamedCastExpr"; */
  case clang::Stmt::CXXStaticCastExprClass: return "CXXStaticCastExpr";
  case clang::Stmt::CXXDynamicCastExprClass: return "CXXDynamicCastExpr";
  case clang::Stmt::CXXReinterpretCastExprClass: return "CXXReinterpretCastExpr";
  case clang::Stmt::CXXConstCastExprClass: return "CXXConstCastExpr";
  case clang::Stmt::CXXFunctionalCastExprClass: return "CXXFunctionalCastExpr";
  case clang::Stmt::CXXTypeidExprClass: return "CXXTypeidExpr";
  case clang::Stmt::UserDefinedLiteralClass: return "UserDefinedLiteral";
  case clang::Stmt::CXXBoolLiteralExprClass: return "CXXBoolLiteralExpr";
  case clang::Stmt::CXXNullPtrLiteralExprClass: return "CXXNullPtrLiteralExpr";
  case clang::Stmt::CXXThisExprClass: return "CXXThisExpr";
  case clang::Stmt::CXXThrowExprClass: return "CXXThrowExpr";
  case clang::Stmt::CXXDefaultArgExprClass: return "CXXDefaultArgExpr";
  case clang::Stmt::CXXDefaultInitExprClass: return "CXXDefaultInitExpr";
  case clang::Stmt::CXXScalarValueInitExprClass: return "CXXScalarValueInitExpr";
  case clang::Stmt::CXXStdInitializerListExprClass: return "CXXStdInitializerListExpr";
  case clang::Stmt::CXXNewExprClass: return "CXXNewExpr";
  case clang::Stmt::CXXDeleteExprClass: return "CXXDeleteExpr";
  case clang::Stmt::CXXPseudoDestructorExprClass: return "CXXPseudoDestructorExpr";
  case clang::Stmt::TypeTraitExprClass: return "TypeTraitExpr";
  case clang::Stmt::ArrayTypeTraitExprClass: return "ArrayTypeTraitExpr";
  case clang::Stmt::ExpressionTraitExprClass: return "ExpressionTraitExpr";
  case clang::Stmt::DependentScopeDeclRefExprClass: return "DependentScopeDeclRefExpr";
  case clang::Stmt::CXXConstructExprClass: return "CXXConstructExpr";
  case clang::Stmt::CXXBindTemporaryExprClass: return "CXXBindTemporaryExpr";
  case clang::Stmt::ExprWithCleanupsClass: return "ExprWithCleanups";
  case clang::Stmt::CXXTemporaryObjectExprClass: return "CXXTemporaryObjectExpr";
  case clang::Stmt::CXXUnresolvedConstructExprClass: return "CXXUnresolvedConstructExpr";
  case clang::Stmt::CXXDependentScopeMemberExprClass: return "CXXDependentScopeMemberExpr";
  /* case clang::Stmt::OverloadExprClass: return "OverloadExpr"; */
  case clang::Stmt::UnresolvedLookupExprClass: return "UnresolvedLookupExpr";
  case clang::Stmt::UnresolvedMemberExprClass: return "UnresolvedMemberExpr";
  case clang::Stmt::CXXNoexceptExprClass: return "CXXNoexceptExpr";
  case clang::Stmt::PackExpansionExprClass: return "PackExpansionExpr";
  case clang::Stmt::SizeOfPackExprClass: return "SizeOfPackExpr";
  case clang::Stmt::SubstNonTypeTemplateParmExprClass: return "SubstNonTypeTemplateParmExpr";
  case clang::Stmt::SubstNonTypeTemplateParmPackExprClass: return "SubstNonTypeTemplateParmPackExpr";
  case clang::Stmt::FunctionParmPackExprClass: return "FunctionParmPackExpr";
  case clang::Stmt::MaterializeTemporaryExprClass: return "MaterializeTemporaryExpr";
  case clang::Stmt::LambdaExprClass: return "LambdaExpr";
  case clang::Stmt::ObjCStringLiteralClass: return "ObjCStringLiteral";
  case clang::Stmt::ObjCBoxedExprClass: return "ObjCBoxedExpr";
  case clang::Stmt::ObjCArrayLiteralClass: return "ObjCArrayLiteral";
  case clang::Stmt::ObjCDictionaryLiteralClass: return "ObjCDictionaryLiteral";
  case clang::Stmt::ObjCEncodeExprClass: return "ObjCEncodeExpr";
  case clang::Stmt::ObjCMessageExprClass: return "ObjCMessageExpr";
  case clang::Stmt::ObjCSelectorExprClass: return "ObjCSelectorExpr";
  case clang::Stmt::ObjCProtocolExprClass: return "ObjCProtocolExpr";
  case clang::Stmt::ObjCIvarRefExprClass: return "ObjCIvarRefExpr";
  case clang::Stmt::ObjCPropertyRefExprClass: return "ObjCPropertyRefExpr";
  case clang::Stmt::ObjCIsaExprClass: return "ObjCIsaExpr";
  case clang::Stmt::ObjCIndirectCopyRestoreExprClass: return "ObjCIndirectCopyRestoreExpr";
  case clang::Stmt::ObjCBoolLiteralExprClass: return "ObjCBoolLiteralExpr";
  case clang::Stmt::ObjCSubscriptRefExprClass: return "ObjCSubscriptRefExpr";
  case clang::Stmt::ObjCBridgedCastExprClass: return "ObjCBridgedCastExpr";
  case clang::Stmt::CUDAKernelCallExprClass: return "CUDAKernelCallExpr";
  case clang::Stmt::ShuffleVectorExprClass: return "ShuffleVectorExpr";
  case clang::Stmt::ConvertVectorExprClass: return "ConvertVectorExpr";
  case clang::Stmt::BlockExprClass: return "BlockExpr";
  case clang::Stmt::OpaqueValueExprClass: return "OpaqueValueExpr";
  case clang::Stmt::MSPropertyRefExprClass: return "MSPropertyRefExpr";
  case clang::Stmt::CXXUuidofExprClass: return "CXXUuidofExpr";
  case clang::Stmt::SEHTryStmtClass: return "SEHTryStmt";
  case clang::Stmt::SEHExceptStmtClass: return "SEHExceptStmt";
  case clang::Stmt::SEHFinallyStmtClass: return "SEHFinallyStmt";
  case clang::Stmt::SEHLeaveStmtClass: return "SEHLeaveStmt";
  case clang::Stmt::MSDependentExistsStmtClass: return "MSDependentExistsStmt";
  case clang::Stmt::AsTypeExprClass: return "AsTypeExpr";
  /* case clang::Stmt::CEANIndexExprClass: return "CEANIndexExpr"; */
  /* case clang::Stmt::OMPExecutableDirectiveClass: return "OMPExecutableDirective"; */
  case clang::Stmt::OMPParallelDirectiveClass: return "OMPParallelDirective";
  case clang::Stmt::OMPParallelForDirectiveClass: return "OMPParallelForDirective";
  case clang::Stmt::OMPParallelForSimdDirectiveClass: return "OMPParallelForSimdDirective";
  case clang::Stmt::OMPForDirectiveClass: return "OMPForDirective";
  /* case clang::Stmt::OMPSimdDirectiveClass: return "OMPSimdDirective"; */
  /* case clang::Stmt::OMPForSimdDirectiveClass: return "OMPForSimdDirective"; */
  /* case clang::Stmt::OMPDistributeSimdDirectiveClass: return "OMPDistributeSimdDirective"; */
  /* case clang::Stmt::OMPDistributeParallelForDirectiveClass: return "OMPDistributeParallelForDirective"; */
  /* case clang::Stmt::OMPDistributeParallelForSimdDirectiveClass: return "OMPDistributeParallelForSimdDirective"; */
  case clang::Stmt::OMPSectionsDirectiveClass: return "OMPSectionsDirective";
  case clang::Stmt::OMPParallelSectionsDirectiveClass: return "OMPParallelSectionsDirective";
  case clang::Stmt::OMPSectionDirectiveClass: return "OMPSectionDirective";
  case clang::Stmt::OMPSingleDirectiveClass: return "OMPSingleDirective";
  case clang::Stmt::OMPTaskDirectiveClass: return "OMPTaskDirective";
  case clang::Stmt::OMPTaskyieldDirectiveClass: return "OMPTaskyieldDirective";
  case clang::Stmt::OMPMasterDirectiveClass: return "OMPMasterDirective";
  case clang::Stmt::OMPCriticalDirectiveClass: return "OMPCriticalDirective";
  case clang::Stmt::OMPBarrierDirectiveClass: return "OMPBarrierDirective";
  case clang::Stmt::OMPTaskwaitDirectiveClass: return "OMPTaskwaitDirective";
  case clang::Stmt::OMPTaskgroupDirectiveClass: return "OMPTaskgroupDirective";
  case clang::Stmt::OMPAtomicDirectiveClass: return "OMPAtomicDirective";
  case clang::Stmt::OMPFlushDirectiveClass: return "OMPFlushDirective";
  case clang::Stmt::OMPOrderedDirectiveClass: return "OMPOrderedDirective";
  /* case clang::Stmt::OMPTeamsDirectiveClass: return "OMPTeamsDirective"; */
  /* case clang::Stmt::OMPDistributeDirectiveClass: return "OMPDistributeDirective"; */
  /* case clang::Stmt::OMPCancelDirectiveClass: return "OMPCancelDirective"; */
  /* case clang::Stmt::OMPCancellationPointDirectiveClass: return "OMPCancellationPointDirective"; */
  /* case clang::Stmt::OMPTargetDirectiveClass: return "OMPTargetDirective"; */
  /* case clang::Stmt::OMPTargetDataDirectiveClass: return "OMPTargetDataDirective"; */
  /* case clang::Stmt::OMPTargetUpdateDirectiveClass: return "OMPTargetUpdateDirective"; */
  /* case clang::Stmt::OMPTargetTeamsDirectiveClass: return "OMPTargetTeamsDirective"; */
  /* case clang::Stmt::OMPTeamsDistributeDirectiveClass: return "OMPTeamsDistributeDirective"; */
  /* case clang::Stmt::OMPTeamsDistributeSimdDirectiveClass: return "OMPTeamsDistributeSimdDirective"; */
  /* case clang::Stmt::OMPTargetTeamsDistributeDirectiveClass: return "OMPTargetTeamsDistributeDirective"; */
  /* case clang::Stmt::OMPTargetTeamsDistributeSimdDirectiveClass: return "OMPTargetTeamsDistributeSimdDirective"; */
  /* case clang::Stmt::OMPTeamsDistributeParallelForDirectiveClass: return "OMPTeamsDistributeParallelForDirective"; */
  /* case clang::Stmt::OMPTeamsDistributeParallelForSimdDirectiveClass: return "OMPTeamsDistributeParallelForSimdDirective"; */
  /* case clang::Stmt::OMPTargetTeamsDistributeParallelForDirectiveClass: return "OMPTargetTeamsDistributeParallelForDirective"; */
  /* case clang::Stmt::OMPTargetTeamsDistributeParallelForSimdDirectiveClass: return "OMPTargetTeamsDistributeParallelForSimdDirective"; */
  default: return "[Unknown OS_type]";
  }
}

struct DDAInfo {
  std::string path;
  std::string filename;
  std::map<unsigned, bool> line_entries;
};

struct FuncStmt {
  std::string func_name;
  unsigned lb_loc;
  unsigned ub_loc;

  FuncStmt(std::string fn, unsigned lb, unsigned ub) {
    func_name = fn;
    lb_loc = lb;
    ub_loc = ub;
  }
};

inline bool operator<(const std::pair<unsigned, OMPStmt>& lhs, const std::pair<unsigned, OMPStmt>& rhs)
{
  return lhs.first < rhs.first;
}

struct OMPInfo {
  std::string path;
  std::string filename;
  // pragma LOC + range of CapturedStmt
  // std::vector<std::pair<unsigned, OMPStmt>> omp_range;
  std::vector<OMPStmt> omp_stmt;
};

struct FuncInfo {
  std::string path;
  std::string filename;
  // pragma LOC + range of CapturedStmt
  // std::vector<std::pair<unsigned, OMPStmt>> omp_range;
  std::vector<FuncStmt> func_stmt;
};

struct LSInfo {
  std::string path;
  std::string filename;
  std::set<std::pair<unsigned, std::string>> line_entries;
};

struct FCInfo {
  std::string path;
  std::string filename;
  std::map<unsigned, std::string> line_entries;
};

struct BLInfo {
  std::string path;
  std::string filename;
  std::set<unsigned> line_entries;
};

struct CompareRange
{
CompareRange(unsigned val) : val_(val) {}
  bool operator()(const std::pair<unsigned, OMPStmt>& elem) const {
    return ((val_ >= elem.second.lb_loc) && (val_ <= elem.second.ub_loc));
  }
private:
  unsigned val_;
};

#endif // COMMON_LLVM_H
