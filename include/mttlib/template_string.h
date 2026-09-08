#ifndef MTTLIB_INCLUDE_TEMPLATE_STRING_H_
#define MTTLIB_INCLUDE_TEMPLATE_STRING_H_

#include <algorithm>
#include <new>
#include <ranges>

#include "box.h"
#include "template_character_span.h"

namespace mttlib {
  template < typename CharacterType >
  class TemplateString {
    static constexpr CharacterType kNullTerminator = kNullTerminator < CharacterType >;
    static constexpr int kAlignment = 16;
    static inline CharacterType kFallback[1]{ kNullTerminator };

    CharacterType * buffer_;
    int size_;
    int buffer_size_;

  public:
    TemplateString(TemplateString const&) = delete;
    TemplateString & operator = (TemplateString const&) = delete;

    static Box < TemplateString > Construct(int buffer_size) noexcept {
      int aligned_buffer_size = AlignValue(buffer_size);
      CharacterType * buffer = Allocate(aligned_buffer_size);

      if (buffer == nullptr) {
        return { };
      }

      buffer[0] = kNullTerminator;

      return TemplateString(buffer, 0, aligned_buffer_size);
    }

    static Box < TemplateString > Construct(CharacterType character, int count) noexcept {
      int aligned_buffer_size = AlignValue(count);
      CharacterType * buffer = Allocate(aligned_buffer_size);

      if (buffer == nullptr) {
        return { };
      }

      std::ranges::fill_n(buffer, count, character);
      buffer[count] = kNullTerminator;

      return TemplateString(buffer, count, aligned_buffer_size);
    }

    static Box < TemplateString > Construct(TemplateCharacterSpan < CharacterType > character_span) noexcept {
      int aligned_buffer_size = AlignValue(character_span.size());
      CharacterType * buffer = Allocate(aligned_buffer_size);

      if (buffer == nullptr) {
        return { };
      }

      std::ranges::copy_n(character_span.pointer(), character_span.size(), buffer);
      buffer[character_span.size()] = kNullTerminator;

      return TemplateString(buffer, character_span.size(), aligned_buffer_size);
    }

    TemplateString() noexcept {
      buffer_ = kFallback;
      size_ = 0;
      buffer_size_ = 0;
    }

    TemplateString(TemplateString && other) noexcept {
      buffer_ = other.buffer_;
      size_ = other.size_;
      buffer_size_ = other.buffer_size_;
      other.buffer_ = nullptr;
    }

    ~TemplateString() {
      Destroy();
    }

    TemplateString & operator = (TemplateString && other) noexcept {
      if (this == &other) {
        return *this;
      }

      Destroy();
      buffer_ = other.buffer_;
      size_ = other.size_;
      buffer_size_ = other.buffer_size_;
      other.buffer_ = nullptr;

      return *this;
    }

    CharacterType const& operator [] (int offset) const noexcept {
      return buffer_[offset];
    }

    CharacterType & operator [] (int offset) noexcept {
      return buffer_[offset];
    }

    CharacterType const* begin() const noexcept {
      return buffer_;
    }

    CharacterType * begin() noexcept {
      return buffer_;
    }

    CharacterType const* end() const noexcept {
      return buffer_ + size_;
    }

    CharacterType * end() noexcept {
      return buffer_ + size_;
    }

    CharacterType const* buffer() const noexcept {
      return buffer_;
    }

    CharacterType * buffer() noexcept {
      return buffer_;
    }

    int size() const noexcept {
      return size_;
    }

    bool Resize(int new_size, CharacterType character) noexcept {
      if (new_size > size_) {
        int aligned_new_buffer_size = AlignValue(new_size);

        if (aligned_new_buffer_size > buffer_size_) {
          CharacterType * new_buffer = Allocate(aligned_new_buffer_size);

          if (new_buffer == nullptr) {
            return false;
          }

          std::ranges::copy_n(buffer_, size_, new_buffer);
          Destroy();
          buffer_ = new_buffer;
          buffer_size_ = aligned_new_buffer_size;
        }

        std::ranges::fill_n(buffer_ + size_, new_size - size_, character);
      }

      buffer_[new_size] = kNullTerminator;
      size_ = new_size;

      return true;
    }

    int buffer_size() const noexcept {
      return buffer_size_;
    }

    bool Reserve(int new_buffer_size) noexcept {
      if (new_buffer_size <= buffer_size_) {
        return true;
      }

      int aligned_new_buffer_size = AlignValue(new_buffer_size);
      CharacterType * new_buffer = Allocate(aligned_new_buffer_size);

      if (new_buffer == nullptr) {
        return false;
      }

      std::ranges::copy_n(buffer_, size_, new_buffer);
      new_buffer[size_] = kNullTerminator;
      Destroy();
      buffer_ = new_buffer;
      buffer_size_ = aligned_new_buffer_size;

      return true;
    }

    bool Append(CharacterType const* characters, int count) noexcept {
      int new_size = size_ + count;
      int aligned_new_buffer_size = AlignValue(new_size);

      if (aligned_new_buffer_size > buffer_size_) {
        CharacterType * new_buffer = Allocate(aligned_new_buffer_size);

        if (new_buffer == nullptr) {
          return false;
        }

        std::ranges::copy_n(buffer_, size_, new_buffer);
        Destroy();
        buffer_ = new_buffer;
        buffer_size_ = aligned_new_buffer_size;
      }

      std::ranges::copy_n(characters, count, buffer_ + size_);
      buffer_[new_size] = kNullTerminator;
      size_ = new_size;

      return true;
    }

    bool Append(CharacterType const* c_string) noexcept {
      return Append(c_string, CStringLength(c_string));
    }

    bool Append(TemplateCharacterSpan < CharacterType > character_span) noexcept {
      return Append(character_span.pointer(), character_span.size());
    }

  private:
    static int AlignValue(int value) noexcept {
      return (value / kAlignment + 1) * kAlignment;
    }

    static CharacterType * Allocate(int count) noexcept {
      return static_cast < CharacterType * > (::operator new(count * sizeof(CharacterType), std::nothrow));
    }

    TemplateString(CharacterType * buffer, int size, int buffer_size) noexcept {
      buffer_ = buffer;
      size_ = size;
      buffer_size_ = buffer_size;
    }

    void Destroy() noexcept {
      ::operator delete(buffer_);
    }
  };

  using String = TemplateString < char >;
  using WideString = TemplateString < wchar_t >;
}

#endif