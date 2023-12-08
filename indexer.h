#pragma once
#include <clang/AST/DeclBase.h>
#include <clang/AST/Expr.h>
#include <clang/Basic/SourceLocation.h>
#include <set>
#include <string>

#include "common.h"

class Indexer {
public:
  Indexer(std::string ProjectFolder) : ProjectFolder(ProjectFolder) { }
  std::string getProjectFolder() { return ProjectFolder; }
  void registerFile(clang::FileID FID);
  std::set<clang::FileID>& getRegisteredFiles() { return RegisteredFiles; }

private:
  std::set<clang::FileID> RegisteredFiles;
  std::string ProjectFolder;
};
