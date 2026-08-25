#ifndef MTTLIB_INCLUDE_BOX_H_
#define MTTLIB_INCLUDE_BOX_H_

#include <type_traits>
#include <utility> // std::forward, std::move

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

    template < typename... ConstructorParametersType >
    explicit Box(BoxConstruct const&, ConstructorParametersType &&... constructor_arguments) noexcept {
      static_assert(std::is_nothrow_constructible_v < ValueType, ConstructorParametersType... >);

      new(&value_) ValueType(std::forward < ConstructorParametersType > (constructor_arguments)...);
      has_value_ = true;
    }

    Box(Box const& other) noexcept {
      static_assert(std::is_nothrow_copy_constructible_v < ValueType >);

      if (other.has_value_) {
        new(&value_) ValueType(other.value_);
      }

      has_value_ = other.has_value_;
    }

    Box(Box && other) noexcept {
      static_assert(std::is_nothrow_move_constructible_v < ValueType >);

      if (other.has_value_) {
        new(&value_) ValueType(std::move(other.value_));
      }

      has_value_ = other.has_value_;
      other.has_value_ = false;
    }

    ~Box() {
      static_assert(std::is_nothrow_destructible_v < ValueType >);

      if (has_value_) {
        value_.~ValueType();
      }
    }

    Box & operator = (Box const& other) noexcept {
      static_assert(std::is_nothrow_copy_assignable_v < ValueType > &&
          std::is_nothrow_destructible_v < ValueType> &&
          std::is_nothrow_copy_constructible_v < ValueType >);

      if (this != &other) {
        if (has_value_) {
          if (other.has_value_) {
            value_ = other.value_;
          }
          else {
            value_.~ValueType();
          }
        }
        else if (other.has_value_) {
          new(&value_) ValueType(other.value_);
        }

        has_value_ = other.has_value_;
      }

      return *this;
    }

    Box & operator = (Box && other) noexcept {
      static_assert(std::is_nothrow_move_assignable_v < ValueType > &&
          std::is_nothrow_destructible_v < ValueType> &&
          std::is_nothrow_move_constructible_v < ValueType >);

      if (this != &other) {
        if (has_value_) {
          if (other.has_value_) {
            value_ = std::move(other.value_);
          }
          else {
            value_.~ValueType();
          }
        }
        else if (other.has_value_) {
          new(&value_) ValueType(std::move(other.value_));
        }

        has_value_ = other.has_value_;
        other.has_value_ = false;
      }

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