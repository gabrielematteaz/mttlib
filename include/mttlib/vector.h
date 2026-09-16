#ifndef MTTLIB_INCLUDE_VECTOR_H_
#define MTTLIB_INCLUDE_VECTOR_H_

#include <new>
#include <type_traits>
#include <utility> // std::move

#include "box.h"
#include "utilities.h"

namespace mttlib {
  template < typename value_type >
  class vector {
    value_type * m_data;
    int m_size;
    int m_capacity;

  public:
    vector(vector const&) = delete;
    vector & operator = (vector const&) = delete;

    static box < vector > construct(int capacity) noexcept {
      int aligned_capacity = align_value(capacity);
      auto data = allocate(aligned_capacity);

      if (data == nullptr) {
        return { };
      }

      return vector(data, 0, aligned_capacity);
    }

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
      m_size = 0;
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
      m_size = 0;

      return *this;
    }

    value_type const* data() const noexcept {
      return m_data;
    }

    value_type * data() noexcept {
      return m_data;
    }

    int size() const noexcept {
      return m_size;
    }

    bool resize(int new_size) noexcept {
      if (new_size <= m_size) {
        destroy(m_data, new_size, m_size - new_size);
      }
      else {
        if (new_size > m_capacity) {
          int new_capacity = align_value(new_size);
          value_type * new_data = allocate(new_capacity);

          if (new_data == nullptr) {
            return false;
          }

          move_construct_uninitialized(m_data, 0, m_size, new_data);
          destroy();
          m_data = new_data;
          m_capacity = new_capacity;
        }

        default_construct_uninitialized(m_data, m_size, new_size - m_size);
      }

      m_size = new_size;

      return true;
    }

    bool resize(int new_size, value_type const& value) noexcept {
      if (new_size <= m_size) {
        destroy(m_data, new_size, m_size - new_size);
      }
      else {
        if (new_size > m_capacity) {
          int new_capacity = align_value(new_size);
          value_type * new_data = allocate(new_capacity);

          if (new_data == nullptr) {
            return false;
          }

          move_construct_uninitialized(m_data, 0, m_size, new_data);
          destroy();
          m_data = new_data;
          m_capacity = new_capacity;
        }

        copy_construct_uninitialized(m_data, m_size, new_size - m_size, value);
      }

      m_size = new_size;

      return true;
    }

    bool empty() const noexcept {
      return m_size == 0;
    }

    int capacity() const noexcept {
      return m_capacity;
    }

  private:
    static value_type * allocate(int count) noexcept {
      return static_cast < value_type * > (::operator new(count * sizeof(value_type),
          std::nothrow));
    }

    static int align_value(int value) noexcept {
      if (value == 0) {
        return 0;
      }

      int aligned_value = 1;

      while (aligned_value < value) {
        aligned_value = aligned_value * 2;
      }

      return aligned_value;
    }

    vector(value_type * data, int size, int capacity) noexcept {
      m_data = data;
      m_size = size;
      m_capacity = capacity;
    }

    void destroy() noexcept {
      destroy(m_data, 0, m_size);
      ::operator delete(m_data);
    }
  };
}

#endif