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

class IndexerVisitor
  : public clang::RecursiveASTVisitor<IndexerVisitor> {
public:
  bool VisitFunctionDecl(clang::FunctionDecl *Decl) {
    if (IndexerDB->shouldIgnore(Decl)) {
      // TODO: For performance purposes, we may want
      // to return false
      return true;
    }
    IndexerDB->addFunctionDecl(Decl);
    return true;
  }

  bool VisitVarDecl(clang::VarDecl *Decl) {
    if (IndexerDB->shouldIgnore(Decl)) {
      // TODO: For performance purposes, we may want
      // to return false
      return true;
    }
    return true;
  }

  bool VisitDeclRefExpr(clang::DeclRefExpr *Expr) {
    if (IndexerDB->shouldIgnore(Expr)) {
      // TODO: For performance purposes, we may want
      // to return false
      return true;
    }
    IndexerDB->addReference(Expr);
    return true;
  }

  void setIndexer(Indexer* IndexerDB) {
    this->IndexerDB = IndexerDB;
  }

private:
  Indexer* IndexerDB;
  // clang::SourceManager* SM;
};

class IndexerConsumer : public clang::ASTConsumer {
public:
  IndexerConsumer(Indexer* IndexerDB) : IndexerDB(IndexerDB) {
    Visitor.setIndexer(IndexerDB);
  }
  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    // Traversing the translation unit decl via a RecursiveASTVisitor
    // will visit all nodes in the AST.
    // auto CurFilename = Context.getSourceManager().getFilename()
    auto &SM = Context.getSourceManager();

    auto FID = SM.getMainFileID();
    const clang::FileEntry *FE = SM.getFileEntryForID(FID);
    auto CurFilename = FE->getName();
    // std::cerr << "FID: " << Context.getSourceManager().getMainFileID().getHashValue() << std::endl;
    // std::cerr << "Filename: " << CurFilename.begin() << std::endl;
    if (CurFilename.rfind(IndexerDB->getProjectFolder()) != 0) {
      std::cerr << "Skipping file " << CurFilename.begin() << " as it is not in the project folder" << std::endl;
      return;
    }

    // TODO: This is wrong, FID and SM will reset for each TU thus the database
    // should be per TU or store files in a different fashion
    IndexerDB->registerFile(FID, &Context.getSourceManager());
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());

    IndexerDB->generateHTML(Context.getSourceManager().getMainFileID());
  }
private:
  Indexer* IndexerDB;
  // A RecursiveASTVisitor implementation.
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
