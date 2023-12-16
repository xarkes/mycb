#pragma once
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <filesystem>
#include <iostream>

#include "indexer.h"
#include "tuindexer.h"

class IndexerVisitor
  : public clang::RecursiveASTVisitor<IndexerVisitor> {
public:
  bool VisitFunctionDecl(clang::FunctionDecl *Decl);
  bool VisitVarDecl(clang::VarDecl *Decl);
  bool VisitDeclRefExpr(clang::DeclRefExpr *Expr);
  bool VisitMemberExpr(clang::MemberExpr *Expr);
  void setIndexer(TUIndexer* TUI) {
    this->TUI = TUI;
  }

private:
  TUIndexer* TUI;
};

class IndexerConsumer : public clang::ASTConsumer {
public:
  IndexerConsumer(Indexer* IndexerDB) : IndexerDB(IndexerDB) { }
  virtual void HandleTranslationUnit(clang::ASTContext &Context);
private:
  Indexer* IndexerDB;
  IndexerVisitor Visitor;
};

class IndexerAction : public clang::ASTFrontendAction {
public:
  IndexerAction(Indexer* IndexerDB) : IndexerDB(IndexerDB) { }
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::make_unique<IndexerConsumer>(IndexerDB);
  }

private:
  Indexer* IndexerDB;
};
