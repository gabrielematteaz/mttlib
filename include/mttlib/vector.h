#ifndef MTTLIB_VECTOR_H_
#define MTTLIB_VECTOR_H_

#include <algorithm>
#include <new>
#include <type_traits>
#include <utility> // std::move, std::forward

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

    // assert(capacity >= 0)
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

    // assert(offset >= 0 && offset < size())
    value_type const& operator [] (int offset) const noexcept {
      return m_data[offset];
    }

    // assert(offset >= 0 && offset < size())
    value_type & operator [] (int offset) noexcept {
      return m_data[offset];
    }

    value_type const* begin() const noexcept {
      return m_data;
    }

    value_type * begin() noexcept {
      return m_data;
    }

    value_type const* end() const noexcept {
      return m_data == nullptr ? nullptr : m_data + m_size;
    }

    value_type * end() noexcept {
      return m_data == nullptr ? nullptr : m_data + m_size;
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

    bool empty() const noexcept {
      return m_size == 0;
    }

    // assert(new_size >= 0)
    bool resize(int new_size) noexcept
    requires std::is_nothrow_default_constructible_v < value_type > {
      if (new_size <= m_size) {
        destroy(m_size, m_size - new_size);
      }
      else {
        if (new_size > m_capacity) {
          int new_capacity = align_value(new_size);
          value_type * new_data = allocate(new_capacity);

          if (new_data == nullptr) {
            return false;
          }

          move_construct(0, m_size, new_data);
          destroy();
          m_data = new_data;
          m_capacity = new_capacity;
        }

        std::ranges::uninitialized_default_construct(m_data + m_size, m_data + new_size);
      }

      m_size = new_size;

      return true;
    }

    // assert(new_size >= 0)
    bool resize(int new_size, value_type const& value) noexcept
    requires std::is_nothrow_copy_constructible_v < value_type > {
      if (new_size <= m_size) {
        destroy(new_size, m_size);
      }
      else {
        if (new_size > m_capacity) {
          int new_capacity = align_value(new_size);
          value_type * new_data = allocate(new_capacity);

          if (new_data == nullptr) {
            return false;
          }

          move_construct(0, m_size, new_data);
          destroy();
          m_data = new_data;
          m_capacity = new_capacity;
        }

        for (auto current = m_data + m_size, last = m_data + new_size; current != last; ++current) {
          ::new(static_cast < void * > (current)) value_type(value);
        }
      }

      m_size = new_size;

      return true;
    }

    int capacity() const noexcept {
      return m_capacity;
    }

    // assert(new_capacity >= 0)
    bool reserve(int new_capacity, bool align = true) noexcept {
      if (align) {
        new_capacity = align_value(new_capacity);
      }

      if (new_capacity <= m_capacity) {
        return true;
      }

      value_type * new_data = allocate(new_capacity);

      if (new_data == nullptr) {
        return false;
      }

      move_construct(0, m_size, new_data);
      destroy();
      m_data = new_data;
      m_capacity = new_capacity;

      return true;
    }

    // insert

    bool push_back(value_type const& value) noexcept
    requires std::is_nothrow_copy_constructible_v < value_type > {
      int new_size = m_size + 1;

      if (new_size > m_capacity) {
        int new_capacity = align_value(new_size);
        value_type * new_data = allocate(new_capacity);

        if (new_data == nullptr) {
          return false;
        }

        move_construct(0, m_size, new_data);
        destroy();
        m_data = new_data;
        m_capacity = new_capacity;
      }

      ::new(static_cast < void * > (m_data + m_size)) value_type(value);
      m_size = new_size;

      return true;
    }

    bool push_back(value_type && value) noexcept
    requires std::is_nothrow_move_constructible_v < value_type > {
      int new_size = m_size + 1;

      if (new_size > m_capacity) {
        int new_capacity = align_value(new_size);
        value_type * new_data = allocate(new_capacity);

        if (new_data == nullptr) {
          return false;
        }

        move_construct(0, m_size, new_data);
        destroy();
        m_data = new_data;
        m_capacity = new_capacity;
      }

      ::new(static_cast < void * > (m_data + m_size)) value_type(std::move(value));
      m_size = new_size;

      return true;
    }

    template < typename... parameters_type >
    bool emplace_back(parameters_type &&... arguments) noexcept
    requires std::is_nothrow_constructible_v < value_type, parameters_type... > {
      int new_size = m_size + 1;

      if (new_size > m_capacity) {
        int new_capacity = align_value(new_size);
        value_type * new_data = allocate(new_capacity);

        if (new_data == nullptr) {
          return false;
        }

        move_construct(0, m_size, new_data);
        destroy();
        m_data = new_data;
        m_capacity = new_capacity;
      }

      ::new(static_cast < void * > (m_data + m_size)) value_type(std::forward < parameters_type > (arguments)...);
      m_size = new_size;

      return true;
    }

    // assert(offset >= 0 && count >= 0 && offset + count <= size())
    void erase(int offset, int count) noexcept
    requires
        std::is_nothrow_move_assignable_v < value_type > &&
        std::is_nothrow_destructible_v < value_type > {
      if (count == 0) {
        return;
      }

      value_type * destination = m_data + offset;
      value_type * source = destination + count;
      value_type * last = m_data + m_size;

      for (; source != last; ++destination, ++source) {
        *destination = *source;
      }

      for (; destination != last; ++ destination) {
        destination->~value_type();
      }

      m_size = m_size - count;
    }

    void clear() noexcept {
      destroy(0, m_size);
      m_size = 0;
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
      return static_cast < value_type * > (::operator new(count * sizeof(value_type), std::nothrow));
    }

    vector(value_type * data, int size, int capacity) noexcept {
      m_data = data;
      m_size = size;
      m_capacity = capacity;
    }

    void destroy(int from_index, int to_index) noexcept
    requires std::is_nothrow_destructible_v < value_type > {
      if (from_index == to_index) {
        return;
      }

      value_type * current = m_data + from_index;
      value_type * last = m_data + to_index;

      do {
        current->~value_type();
        ++current;
      } while (current != last);
    }

    void destroy() noexcept {
      destroy(0, m_size);
      ::operator delete(m_data);
    }

    void move_construct(int from_index, int to_index, value_type * destination) noexcept
    requires std::is_nothrow_move_constructible_v < value_type > {
      if (from_index == to_index) {
        return;
      }

      value_type * current = m_data + from_index;
      value_type * last = m_data + to_index;

      do {
        ::new(static_cast < void * > (destination)) value_type(std::move(*current));
        ++destination;
        ++current;
      } while (current != last);
    }
  };
}

#endif