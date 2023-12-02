#pragma once
#include <clang/Basic/SourceLocation.h>
#include <string>
#include <vector>

class IndexedFile {};

class Indexer {
public:
  Indexer(std::string ProjectFolder) : ProjectFolder(ProjectFolder) { }
  void registerFunction(std::string Name);
  void registerFile(std::string Name);
  std::string getProjectFolder() { return ProjectFolder; }

  bool shouldIgnore(clang::FileID FID);

  void generateHTML(clang::FileID FID);

private:
  std::vector<IndexedFile> Files;
  std::string ProjectFolder;
};
