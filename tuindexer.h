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
    return this->BLine < B.BLine || (this->BLine <= B.BLine && this->BColumn < B.BColumn);
  }
};
std::ostream &operator<<(std::ostream &OS, const Symbol &S);

struct SymRef {
  std::string Name;
  std::string Filename;
};

struct SymDecl {
  std::string Name;
};

class TUIndexer {
public:
  TUIndexer(Indexer* IDB, clang::SourceManager *SM) : IDB(IDB), SM(SM), FID(SM->getMainFileID()) { }
  
  void addFunctionDecl(clang::FunctionDecl *Decl);
  void addVarDecl(clang::VarDecl *Decl);
  void addReference(clang::DeclRefExpr*);
  void addReference(clang::MemberExpr*);

  bool shouldIgnore(clang::Stmt* Stmt);
  bool shouldIgnore(clang::Decl* Decl);
  bool shouldIgnore(clang::FileID FID);

  void generateHTML();

private:
  Indexer* IDB;
  clang::SourceManager* SM;
  clang::FileID FID;

  // std::vector<clang::FunctionDecl*> Functions;
  std::vector<SymDecl> Declarations;
  // std::vector<clang::Expr*> References;
  std::vector<SymRef> References;
  std::set<Symbol> Symbols;
};
