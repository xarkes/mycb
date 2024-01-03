#include "tuindexer.h"
#include <clang/Basic/SourceManager.h>
#include <filesystem>
#include <fstream>

std::ostream &operator<<(std::ostream &OS, const Symbol &S) {
  return OS << "(" << S.BLine << "," << S.BColumn << ") -> (" <<
    S.ELine << ", " << S.EColumn << ") (" <<
    S.Type << ", " << S.Idx << ")";
}

std::string formatDeclName(clang::Decl* Decl) {
  return std::to_string(Decl->getLocation().getHashValue());
}

std::string formatRefName(clang::Expr *Expr) {
  if (clang::isa<clang::DeclRefExpr>(*Expr)) {
    auto *DRef = static_cast<clang::DeclRefExpr*>(Expr);
    return std::to_string(DRef->getFoundDecl()->getLocation().getHashValue());
  }
  ASSERT("Not handled");
  return "";
}

void TUIndexer::addFunctionDecl(clang::FunctionDecl *Decl) {
  auto PLocBegin = SM->getPresumedLoc(Decl->getNameInfo().getBeginLoc());
  auto PLocEnd = SM->getPresumedLoc(Decl->getNameInfo().getEndLoc());
  auto FID = PLocBegin.getFileID();

  Files[FID].addSymbol(Symbol{PLocBegin.getLine(), PLocBegin.getColumn(), PLocEnd.getLine(), PLocEnd.getColumn(), SymType::Function, 0});
  auto RefName = formatDeclName((clang::Decl*) Decl);
  Files[FID].addDecl(SymDecl{RefName});
}

void TUIndexer::addVarDecl(clang::VarDecl *Decl) {
  auto PLocBegin = SM->getPresumedLoc(Decl->getLocation());
  auto PLocEnd = SM->getPresumedLoc(Decl->getSourceRange().getEnd());
  auto FID = PLocBegin.getFileID();

  Files[FID].addSymbol(Symbol{PLocBegin.getLine(), PLocBegin.getColumn(), PLocEnd.getLine(), PLocEnd.getColumn(), SymType::Function, 0});

  auto RefName = formatDeclName((clang::Decl*) Decl);
  Files[FID].addDecl(SymDecl{RefName});
}

void TUIndexer::addReference(clang::DeclRefExpr *Expr) {
  auto PLocBegin = SM->getPresumedLoc(Expr->getSourceRange().getBegin());
  auto PLocEnd = SM->getPresumedLoc(Expr->getSourceRange().getEnd());
  auto FID = PLocBegin.getFileID();
  Files[FID].addSymbol(Symbol{PLocBegin.getLine(), PLocBegin.getColumn(), PLocEnd.getLine(), PLocEnd.getColumn(), SymType::Reference, 0});

  auto FileName = clang::FullSourceLoc(Expr->getDecl()->getBeginLoc(), *SM).getExpansionLoc().getFileEntry()->getName();
  auto RelativeFilename = FileName.substr(IDB->getProjectFolder().length());

  auto Name = formatRefName((clang::Expr*) Expr);
  DBG("DeclRefExpr " << Name << ", " << std::string(RelativeFilename) << " at " << PLocBegin.getLine() << ", " << PLocBegin.getColumn());
  Files[FID].addRef(SymRef{Name, std::string(RelativeFilename)});
}

void TUIndexer::addReference(clang::MemberExpr *Expr) {
  auto PLocBegin = SM->getPresumedLoc(Expr->getMemberLoc());
  auto PLocEnd = SM->getPresumedLoc(Expr->getMemberLoc());
  auto FID = PLocBegin.getFileID();
  Files[FID].addSymbol(Symbol{PLocBegin.getLine(), PLocBegin.getColumn(), PLocEnd.getLine(), PLocEnd.getColumn(), SymType::Reference, 0});

  auto FileName = clang::FullSourceLoc(Expr->getMemberDecl()->getBeginLoc(), *SM).getExpansionLoc().getFileEntry()->getName();
  auto RelativeFilename = FileName.substr(IDB->getProjectFolder().length());

  auto Name = formatRefName((clang::Expr*) Expr);
  DBG("MemberExpr " << Name << ", " << std::string(RelativeFilename) << " at " << PLocBegin.getLine() << ", " << PLocBegin.getColumn());
  Files[FID].addRef(SymRef{Name, std::string(RelativeFilename)});
}

void TUIndexer::addInclude(clang::SourceLocation HashLoc, clang::CharSourceRange FilenameRange, llvm::StringRef FileName) {
  auto FID = SM->getFileID(HashLoc);
  auto PLocBeginLine = SM->getPresumedLineNumber(HashLoc);
  auto PLocBeginCol = SM->getPresumedColumnNumber(HashLoc);

  auto LocBegin = SM->getFileOffset(FilenameRange.getBegin());
  auto LocEnd = SM->getFileOffset(FilenameRange.getEnd());
  auto Length = LocEnd - LocBegin;

  Files[FID].addSymbol(Symbol{PLocBeginLine, PLocBeginCol, PLocBeginLine, PLocBeginCol + Length, SymType::Reference, 0});
  Files[FID].addRef(SymRef{"", std::string(FileName)});
}

bool TUIndexer::shouldIgnore(clang::FileID FID) {
  if (std::find(IDB->getRegisteredFiles().begin(), IDB->getRegisteredFiles().end(), FID) != IDB->getRegisteredFiles().end()) {
    return false;
  }
  return true;
}

bool TUIndexer::shouldIgnore(clang::Decl* Decl) {
  auto FID = clang::FullSourceLoc(Decl->getLocation(), *SM).getExpansionLoc().getFileID();
  return shouldIgnore(FID);
}

bool TUIndexer::shouldIgnore(clang::Stmt* Stmt) {
  auto FID = clang::FullSourceLoc(Stmt->getBeginLoc(), *SM).getExpansionLoc().getFileID();
  return shouldIgnore(FID);
}

void TUIndexer::generateHTML() {
  std::filesystem::path OutFolder = "./out";
  std::string OutputFolder = std::filesystem::absolute(OutFolder);

  auto FID = SM->getMainFileID();
  const clang::FileEntry *FE = SM->getFileEntryForID(FID);
  auto Filename = FE->getName();
  ASSERT(Filename.starts_with(IDB->getProjectFolder()));
  auto RelativeFilename = Filename.substr(IDB->getProjectFolder().length());

  std::string OutputFilename = OutputFolder + "/" + RelativeFilename.str() + ".html";
  auto OutDir = llvm::StringRef(OutputFilename).rsplit('/').first;

  // Force create parent folders
  using namespace llvm::sys::fs;
  auto DefaultPerms = perms::all_all & ~perms::group_write & ~perms::others_write;
  llvm::sys::fs::create_directories(OutDir, true, DefaultPerms);

  // Create output file
  auto FileContent = SM->getBufferData(FID);
  Files[FID].generateHTML(OutputFolder, OutputFilename, FileContent);
  DBG("Files length: " << Files.size());
}

void FileAnnotations::generateHTML(std::string OutputFolder, std::string OutputFilename, llvm::StringRef FileContent) {
  std::ofstream OutputFile;
  OutputFile.open(OutputFilename);

  OutputFile << "<html><head><link rel=\"stylesheet\" href=\"res/style.css\"></head><body><pre>";

  const unsigned char* C = FileContent.bytes_begin();
  int Line = 1;
  int Column = 1;

  auto CurSymbol = Symbols.begin();
  bool WritingSymbol = false;

  while (*C) {
    if (!WritingSymbol && CurSymbol != Symbols.end() && CurSymbol->BLine == Line) {
      if (CurSymbol->BColumn == Column) {
        WritingSymbol = true;
        if (CurSymbol->Type == SymType::Function) {
          // auto *Func = Functions[CurSymbol->Idx];
          // TODO: Mangle for HTML?
          // std::string FuncName = Func->getQualifiedNameAsString();
          auto Decl = Declarations[CurSymbol->Idx];
          OutputFile << "<span class=\"decl\" id=\"" << Decl.Name << "\">";
        } else if (CurSymbol->Type == SymType::Reference) {
          auto Ref = References[CurSymbol->Idx];
          std::string OutputFilename = OutputFolder + "/" + Ref.Filename + ".html";
          OutputFile << "<a href=\"" << OutputFilename << "#" << Ref.Name << "\">";
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
          || *C == ')' || *C == ';') {
        if (CurSymbol->Type == SymType::Function) {
          OutputFile << "</span>";
        } else if (CurSymbol->Type == SymType::Reference) {
          OutputFile << "</a>";
        } else {
          ASSERT(false && "Not handled");
        }
        WritingSymbol = false;
        CurSymbol++;
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
      Line++;
      Column = 0; // will be set to 1 just after
    }
    C++;
    Column++;
  }

  // TODO
  ASSERT(!WritingSymbol);
  ASSERT(CurSymbol == Symbols.end());

  OutputFile << "</pre></body></html>";
  OutputFile.close();
}
