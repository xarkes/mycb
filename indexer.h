#pragma once
#include <clang/AST/DeclBase.h>
#include <clang/Basic/SourceLocation.h>
#include <set>
#include <string>

class Indexer {
public:
  Indexer(std::string ProjectFolder) : ProjectFolder(ProjectFolder) { }
  void registerFile(clang::FileID FID, clang::SourceManager* SM);
  std::string getProjectFolder() { return ProjectFolder; }

  bool shouldIgnore(clang::Decl* Decl);
  bool shouldIgnore(clang::FileID FID);

  void generateHTML(clang::FileID FID);

private:
  std::set<clang::FileID> RegisteredFiles;
  std::string ProjectFolder;

  clang::SourceManager* SM;
};
