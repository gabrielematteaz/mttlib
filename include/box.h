#ifndef MTTLIB_INCLUDE_BOX_H_
#define MTTLIB_INCLUDE_BOX_H_

#include <new>
#include <type_traits>
#include <utility>

namespace mttlib {
  struct BoxConstruct {
    explicit BoxConstruct() = default;
  };

  template < typename ValueType >
  class Box {
    union {
      ValueType value_;
    };

    bool has_value_;
  public:
    Box() noexcept {
      has_value_ = false;
    }

    template < typename ... ConstructorParametersType >
    explicit Box(BoxConstruct const&, ConstructorParametersType &&... constructor_arguments) noexcept {
      static_assert(std::is_nothrow_constructible_v < ValueType, ConstructorParametersType ... >);

      ::new(&value_) ValueType(std::forward < ConstructorParametersType >
          (constructor_arguments) ...);
      has_value_ = true;
    }

    Box(Box const& other) noexcept {
      if (other.has_value_) {
        static_assert(std::is_nothrow_copy_constructible_v < ValueType >);

        ::new(&value_) ValueType(other.value_);
      }

      has_value_ = other.has_value_;
    }

    Box(Box && other) noexcept {
      if (other.has_value_) {
        static_assert(std::is_nothrow_move_constructible_v < ValueType >);

        ::new(&value_) ValueType(std::move(other.value_));
      }

      has_value_ = other.has_value_;
    }

    ~Box() {
      if (has_value_) {
        static_assert(std::is_nothrow_destructible_v < ValueType >);

        value_.~ValueType();
      }
    }

    Box & operator = (Box const& other) noexcept {
      if (this == &other) {
        return *this;
      }

      if (other.has_value_) {
        if (has_value_) {
          static_assert(std::is_nothrow_copy_assignable_v < ValueType >);

          value_ = other.value_;
        }
        else {
          static_assert(std::is_nothrow_copy_constructible_v < ValueType >);

          ::new(&value_) ValueType(other.value_);
        }
      }
      else if (has_value_) {
        static_assert(std::is_nothrow_destructible_v < ValueType >);

        value_.~ValueType();
      }

      has_value_ = other.has_value_;

      return *this;
    }

    Box & operator = (Box && other) noexcept {
      if (this == &other) {
        return *this;
      }

      if (other.has_value_) {
        if (has_value_) {
          static_assert(std::is_nothrow_move_assignable_v < ValueType >);

          value_ = std::move(other.value_);
        }
        else {
          static_assert(std::is_nothrow_move_constructible_v < ValueType >);

          ::new(&value_) ValueType(std::move(other.value_));
        }
      }
      else if (has_value_) {
        static_assert(std::is_nothrow_destructible_v < ValueType >);

        value_.~ValueType();
      }

      has_value_ = other.has_value_;

      return *this;
    }

    explicit operator bool() const noexcept {
      return has_value_;
    }

    ValueType const* operator -> () const noexcept {
      return &value_;
    }

    ValueType * operator -> () noexcept {
      return &value_;
    }

    ValueType const& operator * () const& noexcept {
      return value_;
    }

    ValueType & operator * () & noexcept {
      return value_;
    }

    ValueType const&& operator * () const&& noexcept {
      return value_;
    }

    ValueType && operator * () && noexcept {
      return value_;
    }

    bool has_value() const noexcept {
      return has_value_;
    }

    ValueType const& value() const& noexcept {
      return value_;
    }

    ValueType & value() & noexcept {
      return value_;
    }

    ValueType const&& value() const&& noexcept {
      return value_;
    }

    ValueType && value() && noexcept {
      return value_;
    }
  };

  constexpr BoxConstruct kBoxConstruct;
}

#endif