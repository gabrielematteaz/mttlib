#ifndef MTTLIB_NOTHROW_STRING_H_
#define MTTLIB_NOTHROW_STRING_H_

#include <algorithm>
#include <new>

#include "box.h"
#include "character_span.h"

namespace mttlib {
  class nothrow_string {
    static constexpr int ALIGNMENT = 16;
    static inline char FALLBACK[1]{ '\0' };

    char * m_data;
    int m_size;
    int m_capacity;

  public:
    nothrow_string(nothrow_string const&) = delete;
    nothrow_string & operator = (nothrow_string const&) = delete;

    // assert(capacity >= 0)
    static box < nothrow_string > construct(int capacity) noexcept {
      int aligned_capacity = align_value(capacity);
      char * data = new(std::nothrow) char[aligned_capacity];

      if (data == nullptr) {
        return { };
      }

      data[0] = '\0';

      return nothrow_string(data, 0, aligned_capacity);
    }

    // assert(count >= 0)
    static box < nothrow_string > construct(char character, int count) noexcept {
      int capacity = align_value(count);
      char * data = new(std::nothrow) char[capacity];

      if (data == nullptr) {
        return { };
      }

      std::ranges::fill(data, data + count, character);
      data[count] = '\0';

      return nothrow_string(data, count, capacity);
    }

    // assert(c_string != nullptr)
    static box < nothrow_string > construct(char const* c_string) noexcept {
      int length = mttlib::length(c_string);
      int capacity = align_value(length);
      char * data = new(std::nothrow) char[capacity];

      if (data == nullptr) {
        return { };
      }

      std::ranges::copy(c_string, c_string + length + 1, data);

      return nothrow_string(data, length, capacity);
    }

    // assert(characters != nullptr && count)
    static box < nothrow_string > construct(char const* characters, int count) noexcept {
      int capacity = align_value(count);
      char * data = new(std::nothrow) char[capacity];

      if (data == nullptr) {
        return { };
      }

      std::ranges::copy(characters, characters + count, data);
      data[count] = '\0';

      return nothrow_string(data, count, capacity);
    }

    nothrow_string() noexcept {
      m_data = FALLBACK;
      m_size = 0;
      m_capacity = 0;
    }

    nothrow_string(nothrow_string && other) noexcept {
      m_data = other.m_data;
      m_size = other.m_size;
      m_capacity = other.m_capacity;
      other.m_data = FALLBACK;
    }

    ~nothrow_string() {
      destroy();
    }

    nothrow_string & operator = (nothrow_string && other) noexcept {
      if (this == &other) {
        return *this;
      }

      destroy();
      m_data = other.m_data;
      m_size = other.m_size;
      m_capacity = other.m_capacity;
      other.m_data = FALLBACK;

      return *this;
    }

    explicit operator character_span() const noexcept {
      return { m_data, m_size };
    }

    // assert(offset >= 0 && offset <= size())
    char const& operator [] (int offset) const noexcept {
      return m_data[offset];
    }

    // assert(offset >= 0 && offset <= size())
    char & operator [] (int offset) noexcept {
      return m_data[offset];
    }

    char const* begin() const noexcept {
      return m_data;
    }

    char * begin() noexcept {
      return m_data;
    }

    char const* end() const noexcept {
      return m_data + m_size;
    }

    char * end() noexcept {
      return m_data + m_size;
    }

    char const* data() const noexcept {
      return m_data;
    }

    char * data() noexcept {
      return m_data;
    }

    int size() const noexcept {
      return m_size;
    }

    bool empty() const noexcept {
      return m_size == 0;
    }

    // assert(new_size >= 0)
    bool resize(int new_size, char character = '\0') noexcept {
      if (new_size > m_size) {
        int new_capacity = align_value(new_size);

        if (new_capacity > m_capacity) {
          char * new_data = new(std::nothrow) char[new_capacity];

          if (new_data == nullptr) {
            return false;
          }

          std::ranges::copy(m_data, m_data + m_size, new_data);
          destroy();
          m_data = new_data;
          m_capacity = new_capacity;
        }

        std::ranges::fill(m_data + m_size, m_data + new_size, character);
      }

      m_data[new_size] = '\0';
      m_size = new_size;

      return true;
    }

    int capacity() const noexcept {
      return m_capacity;
    }

    // assert(new_capacity >= 0)
    bool reserve(int new_capacity) noexcept {
      if (new_capacity < m_capacity) {
        return true;
      }

      int aligned_new_capacity = align_value(new_capacity);
      char * new_data = new(std::nothrow) char[aligned_new_capacity];

      if (new_data == nullptr) {
        return false;
      }

      std::ranges::copy(m_data, m_data + m_size + 1, new_data);
      destroy();
      m_data = new_data;
      m_capacity = aligned_new_capacity;

      return true;
    }

    // assert(offset >= 0 && offset <= size() && c_string != nullptr)
    bool insert(int offset, char const* c_string) noexcept {
      return insert(offset, c_string, length(c_string));
    }

    // assert(offset >= 0 && offset <= size() && characters != nullptr && count >= 0)
    bool insert(int offset, char const* characters, int count) noexcept {
      int new_size = m_size + count;
      int new_capacity = align_value(new_size);

      if (new_capacity > m_capacity) {
        char * new_data = new(std::nothrow) char[new_capacity];

        if (new_data == nullptr) {
          return false;
        }

        std::ranges::copy(m_data, m_data + offset, new_data);
        std::ranges::copy(m_data + offset, m_data + m_size + 1, new_data + offset + count);
        destroy();
        m_data = new_data;
        m_capacity = new_capacity;
      }
      else {
        std::ranges::copy_backward(m_data + offset, m_data + m_size + 1, m_data + new_size + 1);
      }

      std::ranges::copy(characters, characters + count, m_data + offset);
      m_size = new_size;

      return true;
    }

    // assert(c_string != nullptr)
    bool append(char const* c_string) noexcept {
      int length = mttlib::length(c_string);
      int new_size = m_size + length;
      int new_capacity = align_value(new_size);

      if (new_capacity > m_capacity) {
        char * new_data = new(std::nothrow) char[new_capacity];

        if (new_data == nullptr) {
          return false;
        }

        std::ranges::copy(m_data, m_data + m_size, new_data);
        destroy();
        m_data = new_data;
        m_capacity = new_capacity;
      }

      std::ranges::copy(c_string, c_string + length + 1, m_data + m_size);
      m_size = new_size;

      return true;
    }

    // assert(characters != nullptr && count >= 0)
    bool append(char const* characters, int count) noexcept {
      int new_size = m_size + count;
      int new_capacity = align_value(new_size);

      if (new_capacity > m_capacity) {
        char * new_data = new(std::nothrow) char[new_capacity];

        if (new_data == nullptr) {
          return false;
        }

        std::ranges::copy(m_data, m_data + m_size, new_data);
        destroy();
        m_data = new_data;
        m_capacity = new_capacity;
      }

      std::ranges::copy(characters, characters + count, m_data + m_size);
      m_data[new_size] = '\0';
      m_size = new_size;

      return true;
    }

    // assert(offset >= 0 && count >= 0 && offset + count <= size())
    void erase(int offset, int count) noexcept {
      std::ranges::copy(m_data + offset + count, m_data + m_size + 1, m_data + offset);
      m_size = m_size - count;
    }

    void clear() noexcept {
      m_data[0] = '\0';
      m_size = 0;
    }

  private:
    static int align_value(int value) noexcept {
      return (value / ALIGNMENT + 1) * ALIGNMENT;
    }

    nothrow_string(char * data, int size, int capacity) noexcept {
      m_data = data;
      m_size = size;
      m_capacity = capacity;
    }

    void destroy() noexcept {
      if (m_data == FALLBACK) {
        return;
      }

      delete[] m_data;
    }
  };
}

#endif