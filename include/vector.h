#ifndef MTTLIB_INCLUDE_VECTOR_H_
#define MTTLIB_INCLUDE_VECTOR_H_

#include <new>
#include <type_traits>
#include <utility>

#include "box.h"

namespace mttlib {
  template < typename ValueType >
  class Vector {
    ValueType * buffer_;
    int size_;
    int buffer_size_;
  
  public:
    Vector(Vector const&) = delete;
    Vector & operator = (Vector const&) = delete;

    static Box < Vector > Construct(int buffer_size) noexcept {
      int aligned_buffer_size = AlignValue(buffer_size);
      ValueType * buffer = Allocate(aligned_buffer_size);

      if (buffer == nullptr) {
        return { };
      }

      return Vector(buffer, 0, aligned_buffer_size);
    }

    Vector() noexcept {
      buffer_ = nullptr;
      size_ = 0;
      buffer_size_ = 0;
    }

    Vector(Vector && other) noexcept {
      buffer_ = other.buffer_;
      size_ = other.size_;
      buffer_size_ = other.buffer_size_;
      other.buffer_ = nullptr;
      other.size_ = 0;
      other.buffer_size_ = 0;
    }

    ~Vector() {
      Destroy();
    }

    Vector & operator = (Vector && other) noexcept {
      if (this == &other) {
        return *this;
      }

      Destroy();
      buffer_ = other.buffer_;
      size_ = other.size_;
      buffer_size_ = other.buffer_size_;
      other.buffer_ = nullptr;
      other.size_ = 0;
      other.buffer_size_ = 0;

      return *this;
    }

    ValueType const& operator [] (int offset) const noexcept {
      return buffer_[offset];
    }

    ValueType & operator [] (int offset) noexcept {
      return buffer_[offset];
    }

    ValueType const* begin() const noexcept {
      return buffer_;
    }

    ValueType * begin() noexcept {
      return buffer_;
    }

    ValueType const* end() const noexcept {
      return buffer_ + size_;
    }

    ValueType * end() noexcept {
      return buffer_ + size_;
    }

    ValueType const* buffer() const noexcept {
      return buffer_;
    }

    ValueType * buffer() noexcept {
      return buffer_;
    }

    int size() const noexcept {
      return size_;
    }

    bool Resize(int new_size) noexcept {
      static_assert(std::is_nothrow_move_constructible_v < ValueType > &&
          std::is_nothrow_default_constructible_v < ValueType >);

      if (new_size <= size_) {
        Destroy(new_size, size_);
      }
      else {
        int aligned_new_buffer_size = AlignValue(new_size);

        if (aligned_new_buffer_size > buffer_size_) {
          ValueType * new_buffer = Allocate(aligned_new_buffer_size);

          if (new_buffer == nullptr) {
            return false;
          }

          for (int index = 0; index < size_; ++index) {
            ::new(new_buffer + index) ValueType(std::move(buffer_[index]));
          }

          Destroy();
          buffer_ = new_buffer;
          buffer_size_ = aligned_new_buffer_size;
        }

        for (int index = size_; index < new_size; ++index) {
          ::new(buffer_ + index) ValueType();
        }
      }

      size_ = new_size;

      return true;
    }

    bool Resize(int new_size, ValueType const& value) noexcept {
      static_assert(std::is_nothrow_move_constructible_v < ValueType > &&
          std::is_nothrow_copy_constructible_v < ValueType >);

      if (new_size <= size_) {
        Destroy(new_size, size_);
      }
      else {
        int aligned_new_buffer_size = AlignValue(new_size);

        if (aligned_new_buffer_size > buffer_size_) {
          ValueType * new_buffer = Allocate(aligned_new_buffer_size);

          if (new_buffer == nullptr) {
            return false;
          }

          for (int index = 0; index < size_; ++index) {
            ::new(new_buffer + index) ValueType(std::move(buffer_[index]));
          }

          Destroy();
          buffer_ = new_buffer;
          buffer_size_ = aligned_new_buffer_size;
        }

        for (int index = size_; index < new_size; ++index) {
          ::new(buffer_ + index) ValueType(value);
        }
      }

      size_ = new_size;

      return true;
    }

    int buffer_size() const noexcept {
      return buffer_size_;
    }

    bool Reserve(int new_buffer_size) noexcept {
      static_assert(std::is_nothrow_move_constructible_v < ValueType >);

      if (new_buffer_size <= buffer_size_) {
        return true;
      }

      int aligned_new_buffer_size = AlignValue(new_buffer_size);
      ValueType * new_buffer = Allocate(aligned_new_buffer_size);

      if (new_buffer == nullptr) {
        return false;
      }

      for (int index = 0; index < size_; ++index) {
        ::new(new_buffer + index) ValueType(std::move(buffer_[index]));
      }

      Destroy();
      buffer_ = new_buffer;
      buffer_size_ = aligned_new_buffer_size;

      return true;
    }

    bool PushBack(ValueType const& value) noexcept {
      static_assert(std::is_nothrow_copy_constructible_v < ValueType > &&
          std::is_nothrow_move_constructible_v < ValueType >);

      int new_size = size_ + 1;
      int aligned_new_buffer_size = AlignValue(new_size);

      if (aligned_new_buffer_size > buffer_size_) {
        ValueType * new_buffer = Allocate(aligned_new_buffer_size);

        if (new_buffer == nullptr) {
          return false;
        }

        for (int index = 0; index < size_; ++index) {
          ::new(new_buffer + index) ValueType(std::move(buffer_[index]));
        }

        Destroy();
        buffer_ = new_buffer;
        buffer_size_ = aligned_new_buffer_size;
      }

      ::new(buffer_ + size_) ValueType(value);
      size_ = new_size;

      return true;
    }

    bool PushBack(ValueType && value) noexcept {
      static_assert(std::is_nothrow_move_constructible_v < ValueType >);

      int new_size = size_ + 1;
      int aligned_new_buffer_size = AlignValue(new_size);

      if (aligned_new_buffer_size > buffer_size_) {
        ValueType * new_buffer = Allocate(aligned_new_buffer_size);

        if (new_buffer == nullptr) {
          return false;
        }

        for (int index = 0; index < size_; ++index) {
          ::new(new_buffer + index) ValueType(std::move(buffer_[index]));
        }

        Destroy();
        buffer_ = new_buffer;
        buffer_size_ = aligned_new_buffer_size;
      }

      ::new(buffer_ + size_) ValueType(std::move(value));
      size_ = new_size;

      return true;
    }

    template < typename ... ConstructorParametersType >
    bool EmplaceBack(ConstructorParametersType &&... constructor_arguments) noexcept {
      static_assert(std::is_nothrow_copy_constructible_v < ValueType > &&
          std::is_nothrow_constructible_v < ValueType, ConstructorParametersType ... >);

      int new_size = size_ + 1;
      int aligned_new_buffer_size = AlignValue(new_size);

      if (aligned_new_buffer_size > buffer_size_) {
        ValueType * new_buffer = Allocate(aligned_new_buffer_size);

        if (new_buffer == nullptr) {
          return false;
        }

        for (int index = 0; index < size_; ++index) {
          ::new(new_buffer + index) ValueType(std::move(buffer_[index]));
        }

        Destroy();
        buffer_ = new_buffer;
        buffer_size_ = aligned_new_buffer_size;
      }

      ::new(buffer_ + size_) ValueType(std::forward < ConstructorParametersType >
          (constructor_arguments) ...);
      size_ = new_size;

      return true;
    }

  private:
    static int AlignValue(int value) noexcept {
      if (value == 0) {
        return 0;
      }

      int aligned_value = 1;

      while (aligned_value < value) {
        aligned_value = aligned_value * 2;
      }

      return aligned_value;
    }

    static ValueType * Allocate(int count) noexcept {
      return static_cast < ValueType * > (::operator new(count * sizeof(ValueType), std::nothrow));
    }

    Vector(ValueType * buffer, int size, int buffer_size) noexcept {
      buffer_ = buffer;
      size_ = size;
      buffer_size_ = buffer_size;
    }

    void Destroy(int from_index, int to_index) noexcept {
      static_assert(std::is_nothrow_destructible_v < ValueType >);

      for (int index = from_index; index < to_index; ++index) {
        buffer_[index].~ValueType();
      }
    }

    void Destroy() noexcept {
      Destroy(0, size_);
      ::operator delete(buffer_);
    }
  };
}

#endif