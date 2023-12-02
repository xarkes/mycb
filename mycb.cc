#include <filesystem>
#include <iostream>
#include <set>

#include "indexer.h"
#include "visitor.h"

int main(int argc, char **argv) {
  // 1. Parse arguments
  std::string ErrorMessage;
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <project_folder>" << std::endl;
    return 1;
  }
  std::filesystem::path ProjFolder = argv[1];
  auto Comp = std::unique_ptr<clang::tooling::CompilationDatabase>(
      clang::tooling::CompilationDatabase::loadFromDirectory(ProjFolder.c_str(),
                                                             ErrorMessage));
  if (!Comp) {
    std::cerr << ErrorMessage;
    return 1;
  }

  // 2. Initialize Clang
  llvm::IntrusiveRefCntPtr<llvm::vfs::OverlayFileSystem> VFS(
      new llvm::vfs::OverlayFileSystem(llvm::vfs::getRealFileSystem()));
  clang::FileManager FM({"."}, VFS);
  FM.Retain();

  // 3. Parse all files
  auto *IndexDB = new Indexer(std::filesystem::absolute(ProjFolder));
  for (auto &Command : Comp->getAllCompileCommands()) {
    std::cerr << "Working with " << Command.Filename << std::endl;
    clang::tooling::ToolInvocation Invocation(
        Command.CommandLine, std::make_unique<IndexerAction>(IndexDB), &FM);
    bool Works = Invocation.run();
    if (!Works) {
      std::cerr << Command.Filename << " was not recognized..." << std::endl;
    }
  }

  return 0;
}
