#include <iostream>

#include "vector.h"

int main() {
  auto integers_box = mttlib::vector < int >::construct(100);

  if (integers_box.has_value() == false) {
    return 1;
  }

  auto & integers = integers_box.value();

  integers.push_back(100);
  integers.push_back(200);
  integers.push_back(300);

  for (int i = 0; i < integers.size(); ++i) {
    std::cout << integers[i] << '\n';
  }
}