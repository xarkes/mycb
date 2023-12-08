#pragma once
#include <clang/AST/DeclBase.h>
#include <clang/AST/Expr.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <set>
#include <string>

#include "indexer.h"
#include "common.h"

enum SymType {
  Function = 0,
  Reference
};

struct Symbol {
  uint32_t BLine;
  uint32_t BColumn;
  uint32_t ELine;
  uint32_t EColumn;
  SymType Type;
  uint32_t Idx;

  bool operator<(Symbol const & B) const {
    return this->BLine < B.BLine;
  }
};

class TUIndexer {
public:
  TUIndexer(Indexer* IDB, clang::SourceManager *SM) : IDB(IDB), SM(SM), FID(SM->getMainFileID()) { }
  
  void addFunctionDecl(clang::FunctionDecl *Decl);
  void addReference(clang::DeclRefExpr*);

  bool shouldIgnore(clang::Stmt* Stmt);
  bool shouldIgnore(clang::Decl* Decl);
  bool shouldIgnore(clang::FileID FID);

  void generateHTML();
  // void generateDB();
  void dumpToDisk();

private:
  Indexer* IDB;
  clang::SourceManager* SM;
  clang::FileID FID;

  std::vector<clang::FunctionDecl*> Functions;
  std::vector<clang::DeclRefExpr*> References;
  std::set<Symbol> Symbols;
};
