#ifndef MTTLIB_INCLUDE_TEMPLATE_STRING_H_
#define MTTLIB_INCLUDE_TEMPLATE_STRING_H_

#include <algorithm>
#include <cassert>
#include <new>
#include <optional>

#include "template_characters_span.h"

namespace mttlib {
  template < typename CharacterType >
  class TemplateString {
  public:
    TemplateString(const TemplateString &) = delete;
    TemplateString &operator = (const TemplateString &) = delete;

    /// assert(buffer_size >= 0)
    static std::optional < TemplateString > Construct(int buffer_size) noexcept {
      assert(buffer_size >= 0);

      int aligned_buffer_size = AlignToBlockSize(buffer_size);
      CharacterType *buffer = new(std::nothrow) CharacterType[aligned_buffer_size];

      if (buffer == nullptr) {
        return std::nullopt;
      }

      buffer[0] = kNullTerminator < CharacterType >;

      return TemplateString(buffer, 0, aligned_buffer_size);
    }

    // assert(count >= 0)
    static std::optional < TemplateString > Construct(CharacterType character,
        int count) noexcept {
      assert(count >= 0);

      int buffer_size = AlignToBlockSize(count);
      CharacterType *buffer = new(std::nothrow) CharacterType[buffer_size];

      if (buffer == nullptr) {
        return std::nullopt;
      }

      std::ranges::fill_n(buffer, count, character);
      buffer[count] = kNullTerminator < CharacterType >;

      return TemplateString(buffer, count, buffer_size);
    }

    // assert(c_string != nullptr)
    static std::optional < TemplateString > Construct(const CharacterType *c_string,
        bool exact) noexcept {
      assert(c_string != nullptr);

      int length = CStringLength(c_string);
      int buffer_size = exact ? length + 1 : AlignToBlockSize(length);
      CharacterType *buffer = new(std::nothrow) CharacterType[buffer_size];

      if (buffer == nullptr) {
        return std::nullopt;
      }

      std::ranges::copy_n(c_string, length + 1, buffer);

      return TemplateString(buffer, length, buffer_size);
    }

    // assert(characters != nullptr && count >= 0)
    static std::optional < TemplateString > Construct(const CharacterType *characters,
        int count, bool exact) noexcept {
      assert(characters != nullptr && count >= 0);

      int buffer_size = exact ? count + 1 : AlignToBlockSize(count);
      CharacterType *buffer = new(std::nothrow) CharacterType[buffer_size];

      if (buffer == nullptr) {
        return std::nullopt;
      }

      std::ranges::copy_n(characters, count, buffer);
      buffer[count] = kNullTerminator < CharacterType >;

      return TemplateString(characters, count, buffer_size);
    }

    TemplateString() noexcept {
      buffer_ = kFallback;
      length_ = 0;
      buffer_size_ = 0;
    }

    TemplateString(TemplateString &&other) noexcept {
      buffer_ = other.buffer_;
      length_ = other.length_;
      buffer_size_ = other.buffer_size_;
      other.buffer_ = other.kFallback;
    }

    ~TemplateString() {
      Destroy();
    }

    TemplateString &operator = (TemplateString &&other) noexcept {
      if (this != &other) {
        Destroy();
        buffer_ = other.buffer_;
        length_ = other.length_;
        buffer_size_ = other.buffer_size_;
        other.buffer_ = other.kFallback;
      }

      return *this;
    }

    operator TemplateCharactersSpan < CharacterType > () const noexcept {
      return { buffer_, size_ };
    }

    const CharacterType *buffer() const noexcept {
      return buffer_;
    }

    CharacterType *buffer() noexcept {
      return buffer_;
    }

    int length() const noexcept {
      return length_;
    }

    // assert(new_length >= 0)
    bool Resize(int new_length, CharacterType character) noexcept {
      assert(new_length >= 0);

      if (new_length > length_) {
        int buffer_size = AlignToBlockSize(new_length);

        if (buffer_size > buffer_size_) {
          CharacterType *buffer = new(std::nothrow) CharacterType[buffer_size];

          if (buffer == nullptr) {
            return false;
          }

          std::ranges::copy_n(buffer_, length_, buffer);
          Destroy();
          buffer_ = buffer;
          buffer_size_ = buffer_size;
        }

        std::ranges::fill_n(buffer_ + length_, new_length - length_, character);
      }

      buffer[new_length] = kNullTerminator < CharacterType >;
      length_ = new_length;

      return true;
    }

    int buffer_size() const noexcept {
      return buffer_size_;
    }

    // assert(new_buffer_size >= 0)
    bool Reserve(int new_buffer_size) noexcept {
      assert(new_buffer_size >= 0);

      int aligned_buffer_size = AlignToBlockSize(new_buffer_size);

      if (aligned_buffer_size > buffer_size_) {
        CharacterType *buffer = new(std::nothrow) CharacterType[aligned_buffer_size];

        if (buffer == nullptr) {
          return false;
        }

        std::ranges::copy_n(buffer_, length_ + 1, buffer);
        Destroy();
        buffer_ = buffer;
        buffer_size_ = new_buffer_size;
      }

      return true;
    }

    // assert(offset >= 0 && offset <= length() && c_string != nullptr)
    bool Insert(int offset, const CharacterType *c_string) noexcept {
      assert(offset >= 0 && offset <= length_ && c_string != nullptr);

      return Insert(offset, c_string, CStringLength(c_string));
    }

    // assert(offset >= 0 && offset <= length() && characters != nullptr && count >= 0)
    bool Insert(int offset, const CharacterType *characters, int count) noexcept {
      assert(offset >= 0 && offset <= length_ && characters != nullptr && count >= 0);

      int new_length = length_ + count;
      int buffer_size = AlignToBlockSize(new_length);

      if (buffer_size > buffer_size_) {
        CharacterType *buffer = new(std::nothrow) CharacterType[buffer_size];

        if (buffer == nullptr) {
          return false;
        }

        std::ranges::copy_n(buffer_, offset, buffer);
        std::ranges::copy_n(buffer_ + offset, length_ - offset, buffer + offset + count);
        Destroy();
        buffer_ = buffer;
        buffer_size_ = buffer_size;
      }
      else {
        std::ranges::copy_backward(buffer_ + offset, buffer_ + length_ + 1, buffer_ + new_length + 1);
      }

      std::ranges::copy_n(characters, count, buffer_ + offset);
      length_ = new_length;

      return true;
    }

    // assert(c_string != nullptr)
    bool Append(const CharacterType *c_string) noexcept {
      assert(c_string != nullptr);

      int length = CStringLength(c_string);
      int new_length = length_ + length;
      int buffer_size = AlignToBlockSize(new_length);

      if (buffer_size > buffer_size_) {
        CharacterType *buffer = new(std::nothrow) CharacterType[buffer_size];

        if (buffer == nullptr) {
          return false;
        }

        std::ranges::copy_n(buffer_, length_, buffer);
        Destroy();
        buffer_ = buffer;
        buffer_size_ = buffer_size_;
      }

      std::ranges::copy_n(c_string, length + 1, buffer_ + length_);
      length_ = new_length;

      return true;
    }

    // assert(characters != nullptr && count >= 0)
    bool Append(const CharacterType *characters, int count) noexcept {
      assert(characters != nullptr && count >= 0);

      int new_length = length_ + count;
      int buffer_size = AlignToBlockSize(new_length);

      if (buffer_size > buffer_size_) {
        CharacterType *buffer = new(std::nothrow) CharacterType[buffer_size];

        if (buffer == nullptr) {
          return false;
        }

        std::ranges::copy_n(buffer_, length_, buffer);
        Destroy();
        buffer_ = buffer;
        buffer_size_ = buffer_size_;
      }

      std::ranges::copy_n(characters, count, buffer_ + length_);
      buffer_[new_length] = kNullTerminator < CharacterType >;
      length_ = new_length;

      return true;
    }

    // assert(offset >= 0 && count >= 0 && offset + count <= length())
    void Erase(int offset, int count) noexcept {
      assert(offset >= 0 && count >= 0 && offset + count <= length_);

      std::ranges::copy_n(buffer_ + offset + count, length_ - offset - count + 1,
          buffer_ + offset);
      length_ = length_ - count;
    }

    // assert(offset >= 0 && offset <= length())
    void Erase(int offset) noexcept {
      assert(offset >= 0 && offset <= length_);

      buffer_[offset] = kNullTerminator < CharacterType >;
      length_ = offset;
    }

    void Clear() noexcept {
      buffer_[0] = kNullTerminator < CharacterType >;
      length_ = 0;
    }

  private:
    static constexpr int kBlockSize = 16;

    static inline CharacterType kFallback[1]{ kNullTerminator < CharacterType > };

    static int AlignToBlockSize(int value) noexcept {
      return (value / kBlockSize + 1) * kBlockSize;
    }

    CharacterType *buffer_;
    int length_;
    int buffer_size_;

    TemplateString(CharacterType *buffer, int length, int buffer_size) noexcept {
      buffer_ = buffer;
      length_ = length;
      buffer_size_ = buffer_size;
    }

    void Destroy() noexcept {
      if (buffer_ != kFallback) {
        delete[] buffer_;
      }
    }
  };

  using String = TemplateString < char >;
  using WideString = TemplateString < wchar_t >;
}

#endif