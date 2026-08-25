#ifndef MTTLIB_INCLUDE_VECTOR_H_
#define MTTLIB_INCLUDE_VECTOR_H_

#include <cassert>
#include <new>
#include <type_traits>
#include <utility> // std::move, std::forward

#include "box.h"

namespace mttlib {
  template < typename ValueType >
  class Vector {
    ValueType * this_values;
    int this_size;
    int this_capacity;

  public:
    Vector(Vector const&) = delete;
    Vector & operator = (Vector const&) = delete;

    static Box < Vector > Construct(int capacity) noexcept {
      assert(capacity >= 0);

      int aligned_capacity = AlignValue(capacity);
      ValueType * values = new(std::nothrow) ValueType[aligned_capacity];

      if (values == nullptr) {
        return { };
      }

      return Box < Vector > (kBoxConstruct, Vector(values, 0, aligned_capacity));
    }

    Vector() noexcept {
      this_values = nullptr;
      this_size = 0;
      this_capacity = 0;
    }

    Vector(Vector && other) noexcept {
      this_values = other.this_values;
      this_size = other.this_size;
      this_capacity = other.this_capacity;
      other.this_values = nullptr;
    }

    ~Vector() {
      Destroy(0, this_size, true);
    }

    Vector & operator = (Vector && other) noexcept {
      if (this != &other) {
        Destroy(0, this_size, true);
        this_values = other.this_values;
        this_size = other.this_size;
        this_capacity = other.this_capacity;
        other.this_values = nullptr;
      }

      return *this;
    }

    ValueType const* values() const noexcept {
      return this_values;
    }

    ValueType * values() noexcept {
      return this_values;
    }

    int size() const noexcept {
      return this_size;
    }

    bool Resize(int new_size) noexcept {
      return InternalResize(new_size);
    }

    bool Resize(int new_size, ValueType const& value) noexcept {
      return InternalResize(new_size, value);
    }

    int capacity() const noexcept {
      return this_capacity;
    }

    bool Reserve(int new_capacity) noexcept {
      assert(new_capacity >= 0);

      static_assert(std::is_nothrow_move_constructible_v < ValueType >);

      int new_aligned_capacity = AlignValue(new_capacity);

      if (new_aligned_capacity > this_capacity) {
        ValueType * new_values = new(std::nothrow) ValueType[new_aligned_capacity];

        if (new_values == nullptr) {
          return false;
        }

        for (int index = 0; index < this_size; ++index) {
          new(new_values + index) ValueType(std::move(this_values[index]));
        }

        Destroy(0, this_size, true);
        this_values = new_values;
        this_capacity = new_aligned_capacity;
      }

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

    Vector(ValueType * values, int size, int capacity) noexcept {
      this_values = values;
      this_size = size;
      this_capacity = capacity;
    }

    void Destroy(int start, int end, bool deallocate) noexcept {
      static_assert(std::is_nothrow_destructible_v < ValueType >);

      for (int index = start; index < end; ++index) {
        this_values[index].~ValueType();
      }

      if (deallocate) {
        delete[] this_values;
      }
    }

    // note: cannot be using with rvalues
    template < typename... ConstructorParametersType >
    bool InternalResize(int new_size, ConstructorParametersType &&... constructor_arguments) noexcept {
      assert(new_size >= 0);

      static_assert(std::is_nothrow_move_constructible_v < ValueType > &&
          std::is_nothrow_constructible_v < ValueType, ConstructorParametersType... >);

      if (new_size < this_size) {
        Destroy(new_size, this_size, false);
      }
      else {
        int new_aligned_capacity = AlignValue(new_size);

        if (new_aligned_capacity > this_capacity) {
          ValueType * new_values = new(std::nothrow) ValueType[new_aligned_capacity];

          if (new_values == nullptr) {
            return false;
          }

          for (int index = 0; index < this_size; ++index) {
            new(new_values + index) ValueType(std::move(this_values[index]));
          }

          Destroy(0, this_size, true);
          this_values = new_values;
          this_capacity = new_aligned_capacity;
        }

        for (int index = this_size; index < new_size; ++index) {
          new(this_values + index)
              ValueType(std::forward < ConstructorParametersType > (constructor_arguments)...);
        }
      }

      this_size = new_size;

      return true;
    }
  };
}

#endif