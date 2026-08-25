#include <print>

#include "vector.h"

int main() {
  auto integers = mttlib::Vector < int >::Construct(12);

  std::println("size: {}\ncapacity: {}", integers->size(), integers->capacity());

  integers->Resize(10, 16);

  std::println("size: {}\ncapacity: {}", integers->size(), integers->capacity());

  integers->Reserve(60);

  std::println("size: {}\ncapacity: {}", integers->size(), integers->capacity());
}