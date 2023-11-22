#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <filesystem>
#include <iostream>
#include <set>

clang::SourceManager *SM = nullptr;
std::filesystem::path ProjFolder;

class IndexerVisitor
  : public clang::RecursiveASTVisitor<IndexerVisitor> {
public:
  bool VisitFunctionDecl(clang::FunctionDecl *Declaration) {
    // Declaration->dump();

    // TODO: Use file ID rather than filename maybe
    // auto FID = clang::FullSourceLoc(Declaration->getLocation(), *SM).getExpansionLoc().getFileID();
    auto CurFilename = Declaration->getLocation().printToString(*SM);
    if (CurFilename.rfind(std::filesystem::absolute(ProjFolder)) != 0) {
      return true;
    }

    std::cerr << Declaration->getNameInfo().getName().getAsString() << " from " << Declaration->getLocation().printToString(*SM) << " is it " << std::endl;
    return true;
  }
};

class IndexerConsumer : public clang::ASTConsumer {
public:
  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    // Traversing the translation unit decl via a RecursiveASTVisitor
    // will visit all nodes in the AST.
    SM = &Context.getSourceManager();
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
private:
  // A RecursiveASTVisitor implementation.
  IndexerVisitor Visitor;
};

class IndexerAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::make_unique<IndexerConsumer>();
  }
};

int main(int argc, char **argv) {
  std::string ErrorMessage;

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <project_folder>" << std::endl;
    return 1;
  }
  ProjFolder = argv[1];
  auto Comp = std::unique_ptr<clang::tooling::CompilationDatabase>(
      clang::tooling::CompilationDatabase::loadFromDirectory(ProjFolder.c_str(),
                                                             ErrorMessage));
  if (!Comp) {
    std::cerr << ErrorMessage;
    return 1;
  }

  llvm::IntrusiveRefCntPtr<llvm::vfs::OverlayFileSystem> VFS(
      new llvm::vfs::OverlayFileSystem(llvm::vfs::getRealFileSystem()));
  clang::FileManager FM({"."}, VFS);
  FM.Retain();

  for (auto &Command : Comp->getAllCompileCommands()) {
    std::cerr << "Working with " << Command.Filename << std::endl;
    clang::tooling::ToolInvocation Invocation(
        Command.CommandLine, std::make_unique<IndexerAction>(), &FM);
    bool Works = Invocation.run();
    if (!Works) {
      std::cerr << Command.Filename << " was not recognized..." << std::endl;
    }
  }

  return 0;
}
