#include <iostream>

#include "vector.h"

class test_t {
  int m_value;

public:
  test_t(int value) noexcept {
    std::cout << "test_t(int), this = " << this << ", m_value = " << value << '\n';
    m_value = value;
  }

  test_t(test_t const& other) noexcept {
    std::cout << "test_t(test_t const&), this = " << this << ", other.m_value = " << other.m_value << '\n';
    m_value = other.m_value;
  }

  test_t(test_t && other) noexcept {
    std::cout << "test_t(test_t &&), this = " << this << ", other.m_value = " << other.m_value << '\n';
    m_value = other.m_value;
  }

  ~test_t() {
    std::cout << "~test_t(), this = " << this << ", m_value = " << m_value << '\n';
  }

  test_t & operator = (test_t const& other) noexcept {
    if (this == &other) {
      return *this;
    }

    std::cout << "test_t & operator = (test_t const&), this = " << this << ", m_value = " << m_value << ", other.m_value = " << other.m_value << '\n';
    m_value = other.m_value;

    return *this;
  }

  test_t & operator = (test_t && other) noexcept {
    if (this == &other) {
      return *this;
    }

    std::cout << "test_t & operator = (test_t &&), this = " << this << ", m_value = " << m_value << ", other.m_value = " << other.m_value << '\n';
    m_value = other.m_value;

    return *this;
  }
};

int main() {
  mttlib::vector < test_t > test_box;

  for (int i = 0; i < 10; ++i) {
    test_box.emplace_back(i);
    std::cout << '\n';
  }

  test_box.erase(2, 2);
}