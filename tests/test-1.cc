#include <print>

#include "vector.h"

int main() {
  mttlib::vector < int > integers;

  integers.push_back(10);
  integers.push_back(20);
  integers.push_back(30);
  integers.push_back(40);
  integers.push_back(50);
  integers.push_back(60);

  std::print("size(): {} capacity(): {}\n{}", integers.size(), integers.capacity(), integers);
}