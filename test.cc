#include "tuindexer.h"

bool test_symbol_compare() {
  Symbol A{0, 0, 0, 0, SymType::Reference, 0};
  Symbol B{0, 0, 0, 0, SymType::Reference, 0};
  Symbol C{1, 0, 0, 0, SymType::Reference, 0};
  Symbol D{1, 5, 0, 0, SymType::Reference, 0};
  Symbol E{1, 10, 0, 0, SymType::Reference, 0};

  ASSERT(!(A < B));
  ASSERT(!(B < A));
  ASSERT(A < C);
  ASSERT(C < D);
  ASSERT(D < E);

  return true;
}

int main() {
  test_symbol_compare();
  return 0;
}
