#include "vector.h"

int main() {
  auto integers = mttlib::vector < int >::construct(100);

  if (integers.has_value() == false) {
    return 1;
  }

  integers->resize(10);
  integers->resize(20, 10);
}