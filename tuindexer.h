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

class FileAnnotations {
private:
  Indexer* IDB;
public:
  void addSymbol(Symbol Sym) {
    if (Sym.Type == SymType::Function) {
      Sym.Idx = Declarations.size();   
    } else if (Sym.Type == SymType::Reference) {
      Sym.Idx = References.size();
    }
    Symbols.emplace(Sym);
  }
  void addDecl(SymDecl Decl) { Declarations.emplace_back(Decl); }
  void addRef(SymRef Ref) { References.emplace_back(Ref); }

  void generateHTML(std::string OutputFolder, std::string OutputFilename, llvm::StringRef FileContent);

private:
  // Main data structures for our code references and such
  std::vector<SymDecl> Declarations;
  std::vector<SymRef> References;
  std::set<Symbol> Symbols;
};

class TUIndexer {
public:
  TUIndexer() { }

  void initialize(Indexer* IDB, clang::SourceManager *SM) {
    this->IDB = IDB;
    this->SM = SM;
  }

  void addFunctionDecl(clang::FunctionDecl *Decl);
  void addVarDecl(clang::VarDecl *Decl);
  void addReference(clang::DeclRefExpr*);
  void addReference(clang::MemberExpr*);
  void addInclude(clang::SourceLocation HashLoc, clang::CharSourceRange FilenameRange, llvm::StringRef FileName);

  bool shouldIgnore(clang::Stmt* Stmt);
  bool shouldIgnore(clang::Decl* Decl);
  bool shouldIgnore(clang::FileID FID);

  void generateHTML();

private:
  Indexer* IDB = nullptr;
  clang::SourceManager* SM = nullptr;

  std::map<clang::FileID, FileAnnotations> Files;
};
