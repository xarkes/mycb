#include "indexer.h"

void Indexer::registerFile(clang::FileID FID, clang::SourceManager* SM) {
  RegisteredFiles.emplace(FID);
  this->SM = SM;
}

bool Indexer::shouldIgnore(clang::FileID FID) {
  if (std::find(RegisteredFiles.begin(), RegisteredFiles.end(), FID) != RegisteredFiles.end()) {
    return false;
  }
  return true;
}

bool Indexer::shouldIgnore(clang::Decl* Decl) {
  auto FID = clang::FullSourceLoc(Decl->getLocation(), *SM).getExpansionLoc().getFileID();
  return shouldIgnore(FID);
}
