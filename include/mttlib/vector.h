#ifndef MTTLIB_VECTOR_H_
#define MTTLIB_VECTOR_H_

#include <type_traits>
#include <memory>

namespace mttlib {
  template < typename value_type >
  class vector {
    value_type * m_data;
    int m_size;
    int m_capacity;

  public:
    vector(vector const&) = delete;
    vector & operator = (vector const&) = delete;

    vector() noexcept {
      m_data = nullptr;
      m_size = 0;
      m_capacity = 0;
    }

    vector(vector && other) noexcept {
      m_data = other.m_data;
      m_size = other.m_size;
      m_capacity = other.m_capacity;
      other.m_data = nullptr;
      other.m_size = 0;
    }

    ~vector() {
      destroy();
    }

    vector & operator = (vector && other) noexcept {
      if (this == &other) {
        return *this;
      }

      destroy();
      m_data = other.m_data;
      m_size = other.m_size;
      m_capacity = other.m_capacity;
      other.m_data = nullptr;
      other.m_size = 0;

      return *this;
    }

  private:
    void destroy(int from_index, int to_index) noexcept
    requires std::is_nothrow_destructible_v < value_type > {
      if (from_index == to_index) {
        return;
      }

      value_type * current = m_data + from_index;
      value_type * last = m_data + to_index;

      do {
        std::ranges::destroy_at(current);
        ++current;
      } while (current != last);
    }
  };
}

#endif