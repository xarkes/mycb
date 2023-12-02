#include "indexer.h"
#include <clang/Basic/SourceManager.h>
#include <fstream>

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

void Indexer::generateHTML(clang::FileID FID) {
  std::string OutputFolder = "./out";



  const clang::FileEntry *FE = SM->getFileEntryForID(FID);
  auto Filename = FE->getName();
  ASSERT(Filename.starts_with(ProjectFolder));
  auto RelativeFilename = Filename.substr(ProjectFolder.length());

  std::string OutputFilename = OutputFolder + "/" + RelativeFilename.str() + ".html";
  auto OutDir = llvm::StringRef(OutputFilename).rsplit('/').first;

  // Force create parent folders
  using namespace llvm::sys::fs;
  auto DefaultPerms = perms::all_all & ~perms::group_write & ~perms::others_write;
  llvm::sys::fs::create_directories(OutDir, true, DefaultPerms);

  // Create output file
  std::ofstream OutputFile;
  OutputFile.open(OutputFilename);

  auto FileContent = SM->getBufferData(FID);

  OutputFile << "<html><pre>";
  OutputFile << FileContent.bytes_begin();
  OutputFile << "</pre></html>";
  OutputFile.close();
}
