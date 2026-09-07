#ifndef MTTLIB_INCLUDE_POINTER_H_
#define MTTLIB_INCLUDE_POINTER_H_

#include <new>
#include <type_traits>
#include <utility>

namespace mttlib {
  template < typename ValueType >
  class Pointer {
    ValueType * value_;

  public:
    Pointer(Pointer const&) = delete;
    Pointer & operator = (Pointer const&) = delete;
    
    Pointer() noexcept {
      value_ = nullptr;
    }

    template < typename RealValueType = ValueType, typename ... ConstructorParametersType >
    static Pointer Construct(ConstructorParametersType &&... constructor_arguments) noexcept {
      static_assert(std::is_base_of_v < ValueType, RealValueType > &&
          std::is_nothrow_constructible_v < RealValueType, ConstructorParametersType ... >);

      return new(std::nothrow) RealValueType(std::forward < ConstructorParametersType >
          (constructor_arguments) ...);
    }

    Pointer(Pointer && other) noexcept {
      value_ = other.Release();
    }

    ~Pointer() {
      Destroy();
    }

    Pointer & operator = (Pointer && other) noexcept {
      if (this == &other) {
        return *this;
      }

      Destroy();
      value_ = other.Release();

      return *this;
    }

    explicit operator bool() const noexcept {
      return value_ != nullptr;
    }

    ValueType * operator -> () const noexcept {
      return value_;
    }

    ValueType & operator * () const noexcept {
      return *value_;
    }

    ValueType * get() const noexcept {
      return value_;
    }

    ValueType * Release() noexcept {
      ValueType * value = value_;

      value_ = nullptr;

      return value;
    }

  private:
    Pointer(ValueType * value) noexcept {
      value_ = value;
    }

    void Destroy() noexcept {
      delete value_;
    }
  };
}

#endif