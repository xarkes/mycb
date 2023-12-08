#pragma once
#include <clang/AST/DeclBase.h>
#include <clang/AST/Expr.h>
#include <clang/Basic/SourceLocation.h>
#include <set>
#include <string>

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
    // return this->BLine > B.BLine && this->BColumn > B.BColumn;
    // return this->BLine < B.BLine && this->BColumn < B.BColumn;
  }
};

class Indexer {
public:
  Indexer(std::string ProjectFolder) : ProjectFolder(ProjectFolder) { }
  std::string getProjectFolder() { return ProjectFolder; }

  void registerFile(clang::FileID FID, clang::SourceManager* SM);
  void addFunctionDecl(clang::FunctionDecl *Decl);
  void addReference(clang::DeclRefExpr*);

  bool shouldIgnore(clang::Stmt* Stmt);
  bool shouldIgnore(clang::Decl* Decl);
  bool shouldIgnore(clang::FileID FID);

  void generateHTML(clang::FileID FID);
  // void generateDB();
  void dumpToDisk(clang::FileID FID);

  void clear() { Functions.clear(); References.clear(); Symbols.clear(); }

private:
  std::set<clang::FileID> RegisteredFiles;
  std::string ProjectFolder;

  std::vector<clang::FunctionDecl*> Functions;
  std::vector<clang::DeclRefExpr*> References;

  std::set<Symbol> Symbols;

  clang::SourceManager* SM;
};
