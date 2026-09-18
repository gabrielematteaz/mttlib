#ifndef MTTLIB_INCLUDE_VECTOR_H_
#define MTTLIB_INCLUDE_VECTOR_H_

#include <new>
#include <type_traits>

#include "box.h"

namespace mttlib {
  template < typename value_type >
  class vector {
    value_type * m_data;
    int m_size;
    int m_capacity;

  public:
    vector(vector const&) = delete;
    vector & operator = (vector const&) = delete;

    static box < vector > construct(int capacity, bool align = true) noexcept {
      if (align) {
        capacity = align_value(capacity);
      }

      value_type * data = allocate(capacity);

      if (data == nullptr) {
        return { };
      }

      return vector(data, 0, capacity);
    }

    vector() {
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

    value_type const& operator [] (int offset) const noexcept {
      return m_data[offset];
    }

    value_type & operator [] (int offset) noexcept {
      return m_data[offset];
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

    int resize(int new_size) noexcept {
      if (new_size <= m_size) {
        destroy(new_size, m_size - new_size);
      }
      else {
        if (new_size > m_capacity) {
          int new_capacity = align_value(new_size);
          value_type * new_data = allocate(new_capacity);

          if (new_data == nullptr) {
            return 0;
          }

          move_construct_uninitialized(0, m_size, new_data);
          destroy();
          m_data = new_data;
          m_capacity = new_capacity;
        }

        default_construct_uninitialized(m_size, new_size - m_size);
      }

      m_size = new_size;

      return m_capacity;
    }

    int resize(int new_size, value_type const& value) noexcept {
      if (new_size <= m_size) {
        destroy(new_size, m_size - new_size);
      }
      else {
        if (new_size > m_capacity) {
          int new_capacity = align_value(new_size);
          value_type * new_data = allocate(new_capacity);

          if (new_data == nullptr) {
            return 0;
          }

          move_construct_uninitialized(0, m_size, new_data);
          destroy();
          m_data = new_data;
          m_capacity = new_capacity;
        }

        copy_construct_uninitialized(m_size, new_size - m_size, value);
      }

      m_size = new_size;

      return m_capacity;
    }

    int capacity() const noexcept {
      return m_capacity;
    }

    int reserve(int new_capacity, bool align = true) noexcept {
      if (new_capacity <= m_capacity) {
        return m_capacity;
      }

      if (align) {
        new_capacity = align_value(new_capacity);
      }

      value_type * new_data = allocate(new_capacity);

      if (new_data == nullptr) {
        return 0;
      }

      move_construct_uninitialized(0, m_size, new_data);
      destroy();
      m_data = new_data;
      m_capacity = new_capacity;

      return new_capacity;
    }

    int push_back(value_type const& value) noexcept
    requires std::is_nothrow_copy_constructible_v < value_type > {
      int new_size = m_size + 1;

      if (new_size > m_capacity) {
        int new_capacity = align_value(new_size);
        value_type * new_data = allocate(new_capacity);

        if (new_data == nullptr) {
          return 0;
        }

        move_construct_uninitialized(0, m_size, new_data);
        destroy();
        m_data = new_data;
        m_capacity = new_capacity;
      }

      ::new(static_cast < void * > (m_data + m_size)) value_type(value);
      m_size = new_size;

      return m_capacity;
    }

    int push_back(value_type && value) noexcept
    requires std::is_nothrow_move_constructible_v < value_type > {
      int new_size = m_size + 1;

      if (new_size > m_capacity) {
        int new_capacity = align_value(new_size);
        value_type * new_data = allocate(new_capacity);

        if (new_data == nullptr) {
          return 0;
        }

        move_construct_uninitialized(0, m_size, new_data);
        destroy();
        m_data = new_data;
        m_capacity = new_capacity;
      }

      ::new(static_cast < void * > (m_data + m_size)) value_type(std::move(value));
      m_size = new_size;

      return m_capacity;
    }

  private:
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

    static value_type * allocate(int count) noexcept {
      return static_cast < value_type * > (::operator new(count * sizeof(value_type),
          std::nothrow));
    }

    vector(value_type * data, int size, int capacity) noexcept {
      m_data = data;
      m_size = size;
      m_capacity = capacity;
    }

    void copy_construct_uninitialized(int offset, int count, value_type const& value) noexcept
    requires std::is_nothrow_copy_constructible_v < value_type > {
      if (count == 0) {
        return;
      }

      value_type * current = m_data + offset;

      do {
        ::new(static_cast < void * > (current)) value_type(value);
        ++current;
        --count;
      } while (count != 0);
    }

    void default_construct_uninitialized(int offset, int count) noexcept
    requires std::is_nothrow_default_constructible_v < value_type > {
      if (count == 0) {
        return;
      }

      value_type * current = m_data + offset;

      do {
        ::new(static_cast < void * > (current)) value_type();
        ++current;
        --count;
      } while (count != 0);
    }

    void destroy(int offset, int count) noexcept
    requires std::is_nothrow_destructible_v < value_type > {
      if (count == 0) {
        return;
      }

      value_type * current = m_data + offset;

      do {
        current->~value_type();
        ++current;
        --count;
      } while (count != 0);
    }

    void destroy() noexcept {
      destroy(0, m_size);
      ::operator delete(m_data);
    }

    void move_construct_uninitialized(int offset, int count, value_type * destination) noexcept
    requires std::is_nothrow_move_constructible_v < value_type > {
      if (count == 0) {
        return;
      }

      value_type * current = m_data + offset;

      do {
        ::new(static_cast < void * > (destination)) value_type(std::move(*current));
        ++destination;
        ++current;
        --count;
      } while (count != 0);
    }
  };
}

#endif
