#include "visitor.h"

bool IndexerVisitor::VisitFunctionDecl(clang::FunctionDecl *Decl) {
  if (TUI->shouldIgnore(Decl)) {
    // TODO: For performance purposes, we may want
    // to return false
    return true;
  }
  TUI->addFunctionDecl(Decl);
  return true;
}

bool IndexerVisitor::VisitVarDecl(clang::VarDecl *Decl) {
  if (TUI->shouldIgnore(Decl)) {
    // TODO: For performance purposes, we may want
    // to return false
    return true;
  }
  return true;
}

bool IndexerVisitor::VisitDeclRefExpr(clang::DeclRefExpr *Expr) {
  if (TUI->shouldIgnore(Expr)) {
    // TODO: For performance purposes, we may want
    // to return false
    return true;
  }
  TUI->addReference(Expr);
  return true;
}

// void setIndexer(Indexer* IndexerDB) {
//   this->IndexerDB = IndexerDB;
// }

void IndexerConsumer::HandleTranslationUnit(clang::ASTContext &Context) {
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

  IndexerDB->registerFile(FID);
  auto TUI = TUIndexer(IndexerDB, &SM);
  Visitor.setIndexer(&TUI);
  Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  TUI.generateHTML();
  TUI.dumpToDisk();
}
