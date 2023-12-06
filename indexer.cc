#include "indexer.h"
#include <clang/Basic/SourceManager.h>
#include <fstream>
#include <filesystem>

void Indexer::registerFile(clang::FileID FID, clang::SourceManager* SM) {
  RegisteredFiles.emplace(FID);
  this->SM = SM;
}

void Indexer::addFunctionDecl(clang::FunctionDecl *Decl) {
  // auto PLocBegin = SM->getPresumedLoc(Decl->getSourceRange().getBegin());
  // auto PLocEnd = SM->getPresumedLoc(Decl->getSourceRange().getEnd());

  auto PLocBegin = SM->getPresumedLoc(Decl->getNameInfo().getBeginLoc());
  auto PLocEnd = SM->getPresumedLoc(Decl->getNameInfo().getEndLoc());

  Symbols.emplace(Symbol{PLocBegin.getLine(), PLocBegin.getColumn(), PLocEnd.getLine(), PLocEnd.getColumn(), SymType::Function, (uint32_t) Functions.size()});
  Functions.emplace_back(Decl);

  DBG("Added function for " << PLocBegin.getLine() << "," << PLocBegin.getColumn() << " to " << PLocEnd.getLine() << "," << PLocEnd.getColumn());
  DBG("   " << Decl->getNameInfo().getName().getAsString());
}

void Indexer::addReference(clang::DeclRefExpr *Expr) {
  auto PLocBegin = SM->getPresumedLoc(Expr->getSourceRange().getBegin());
  auto PLocEnd = SM->getPresumedLoc(Expr->getSourceRange().getEnd());
  Symbols.emplace(Symbol{PLocBegin.getLine(), PLocBegin.getColumn(), PLocEnd.getLine(), PLocEnd.getColumn(), SymType::Reference, (uint32_t) References.size()});
  References.emplace_back(Expr);

  DBG("Added for " << PLocBegin.getLine() << "," << PLocBegin.getColumn() << " to " << PLocEnd.getLine() << "," << PLocEnd.getColumn());
  DBG("   " << Expr->getNameInfo().getName().getAsString());
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

bool Indexer::shouldIgnore(clang::Stmt* Stmt) {
  auto FID = clang::FullSourceLoc(Stmt->getBeginLoc(), *SM).getExpansionLoc().getFileID();
  return shouldIgnore(FID);
}

void Indexer::generateHTML(clang::FileID FID) {
  std::filesystem::path OutFolder = "./out";
  std::string OutputFolder = std::filesystem::absolute(OutFolder);

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

  OutputFile << "<html><style>.func:target { background-color: yellow; }</style><pre>";

  const unsigned char* C = FileContent.bytes_begin();
  int Line = 1;
  int Column = 1;

  auto CurSymbol = Symbols.begin();
  bool WritingSymbol = false;
  DBG("Next symbol: " << CurSymbol->BLine << "," << CurSymbol->BColumn);

  while (*C) {
    if (!WritingSymbol && CurSymbol != Symbols.end() && CurSymbol->BLine == Line) {
      DBG("Symbol ok " << Line << "," << Column);
      if (CurSymbol->BColumn == Column) {
        WritingSymbol = true;
        if (CurSymbol->Type == SymType::Function) {
          auto *Func = Functions[CurSymbol->Idx];
          std::string FuncName = Func->getNameAsString();
          OutputFile << "<span class=\"func\" id=\"" << FuncName << "\">";
        } else if (CurSymbol->Type == SymType::Reference) {
          auto *Ref = References[CurSymbol->Idx];
          auto FileName = clang::FullSourceLoc(Ref->getBeginLoc(), *SM).getExpansionLoc().getFileEntry()->getName();
          auto RelativeFilename = FileName.substr(ProjectFolder.length());
          std::string OutputFilename = OutputFolder + "/" + RelativeFilename.str() + ".html";

          std::string SymName = Ref->getNameInfo().getAsString();
          OutputFile << "<a href=\"" << OutputFilename << "#" << SymName << "\">";
        } else {
          ASSERT(false && "Not handled");
        }
      }
    }

    // Stop the symbol writing on the next word as it seems
    // the location for symbol end is not properly computed
    // XXX(1): Verify if we cannot fix the location/have something more accurate
    if (WritingSymbol) {
      if (*C == '.' || *C == '-' || *C == '\t' || *C == ' ' || *C == '('
          || *C == ';') {
        if (CurSymbol->Type == SymType::Function) {
          OutputFile << "</span>";
        } else if (CurSymbol->Type == SymType::Reference) {
          OutputFile << "</a>";
        } else {
          ASSERT(false && "Not handled");
        }
        WritingSymbol = false;
        CurSymbol++;
        DBG("Next symbol: " << CurSymbol->BLine << "," << CurSymbol->BColumn);
      }
    }

    // HtmlEntities
    if (*C == '<') {
      OutputFile << "&lt;";
    } else if (*C == '>') {
      OutputFile << "&gt;";
    } else {
      OutputFile << *C;
    }

    /// XXX: Cf XXX(1)
    // if (WritingSymbol && CurSymbol->ELine == Line) {
    //   DBG("Symbol end? " << Line << "," << Column);
    //   if (CurSymbol->EColumn == Column) {
    //     OutputFile << "</a>";
    //     WritingSymbol = false;
    //     CurSymbol++;
    //     DBG("Next symbol: " << CurSymbol->BLine << "," << CurSymbol->BColumn);
    //   }
    // }

    if (*C == '\n') {
      DBG("Line: " << Line);
      Line++;
      Column = 0; // will be set to 1 just after
    }
    C++;
    Column++;
  }

  // TODO
  // ASSERT(!WritingSymbol);
  // ASSERT(CurSymbol == Symbols.end());

  OutputFile << "</pre></html>";
  OutputFile.close();
}
