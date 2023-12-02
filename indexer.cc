#include "indexer.h"

bool Indexer::shouldIgnore(clang::FileID FID) {
  // auto CurFilename = Decl->getLocation().printToString(*SM);
  // if (CurFilename.rfind(std::filesystem::absolute(ProjFolder)) != 0) {
  //   return true;
  // }
  return true;
}
