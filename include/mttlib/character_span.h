#ifndef MTTLIB_CHARACTER_SPAN_H_
#define MTTLIB_CHARACTER_SPAN_H_

#include <cstdint>

namespace mttlib {
  // assert(c_string != nullptr)
  inline int length(char const* c_string) noexcept {
    char const* current = c_string;

    while (*current != '\0') {
      ++current;
    }

    return static_cast < int > (current - c_string);
  }

  class character_span {
    char const* m_data;
    int m_size;

  public:
    character_span() noexcept {
      m_data = nullptr;
      m_size = 0;
    }

    // assert(c_string != nullptr)
    character_span(char const* c_string) noexcept {
      m_data = c_string;
      m_size = length(c_string);
    }

    // assert(count == 0 || characters != nullptr && count > 0)
    character_span(char const* characters, int count) noexcept {
      m_data = characters;
      m_size = count;
    }

    // assert(offset >= 0 && offset < size())
    char const& operator [] (int offset) const noexcept {
      return m_data[offset];
    }

    char const* begin() const noexcept {
      return m_data;
    }

    char const* end() const noexcept {
      return m_data == nullptr ? nullptr : m_data + m_size;
    }

    char const* data() const noexcept {
      return m_data;
    }

    int size() const noexcept {
      return m_size;
    }

    bool empty() const noexcept {
      return m_size == 0;
    }
  };

  // FNV-1a
  struct case_sensitive_hasher_character_span {
    std::uint64_t operator () (character_span const& value) const noexcept {
      std::uint64_t result = 14695981039346656037ull;

      for (int i = 0; i < value.size(); ++i) {
        result = result ^ static_cast < std::uint8_t > (value[i]);
        result = result * 1099511628211ull;
      }

      return result;
    }
  };
}

#endif