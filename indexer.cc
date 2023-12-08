#include "indexer.h"
#include <clang/Basic/SourceManager.h>
#include <filesystem>
#include <fstream>

void Indexer::registerFile(clang::FileID FID) {
  RegisteredFiles.emplace(FID);
}
