#ifndef MTTLIB_INCLUDE_BOX_H_
#define MTTLIB_INCLUDE_BOX_H_

#include <type_traits>
#include <utility>

namespace mttlib {
  struct BoxConstruct { };

  template < typename Type >
  class Box {
    union {
      Type value_;
    };
    bool has_value_;

  public:
    Box() noexcept {
      has_value_ = false;
    }

    template < typename... ConstructorParametersType >
    explicit Box(const BoxConstruct &, ConstructorParametersType&&...
        constructor_arguments) noexcept {
      new(&value_) Type(std::forward < ConstructorParametersType >
          (constructor_arguments)...);
      has_value_ = true;
    }

    Box(const Box &other) noexcept {
      static_assert(std::is_nothrow_copy_constructible_v < Type >);

      if (other.has_value_) {
        new(&value_) Type(other.value_);
      }

      has_value_ = other.has_value_;
    }

    Box(Box &&other) noexcept {
      static_assert(std::is_nothrow_move_constructible_v < Type >);

      if (other.has_value_) {
        new(&value_) Type(std::move(other.value_));
      }

      has_value_ = other.has_value_;
      other.has_value_ = false;
    }

    ~Box() {
      if (has_value_) {
        value_.~Type();
      }
    }

    Box &operator = (const Box &other) noexcept {
      static_assert(std::is_nothrow_copy_assignable_v < Type > &&
          std::is_nothrow_copy_constructible_v < Type >);

      if (this != &other) {
        if (other.has_value_) {
          if (has_value_) {
            value_ = other.has_value_;
          }
          else {
            new(&value_) Type(other.value_);
          }
        }
        else if (has_value_) {
          value_.~Type();
        }

        has_value_ = other.has_value_;
      }

      return *this;
    }

    Box &operator = (Box &&other) noexcept {
      static_assert(std::is_nothrow_move_assignable_v < Type > &&
          std::is_nothrow_move_constructible_v < Type >);

      if (this != &other) {
        if (other.has_value_) {
          if (has_value_) {
            value_ = std::move(other.has_value_);
          }
          else {
            new(&value_) Type(std::move(other.value_));
          }
        }
        else if (has_value_) {
          value_.~Type();
        }

        has_value_ = other.has_value_;
        other.has_value_ = false;
      }

      return *this;
    }

    const Type &operator * () const & noexcept {
      return value_;
    }

    Type &operator * () & noexcept {
      return value_;
    }

    const Type &&operator * () const && noexcept {
      return value_;
    }

    Type &&operator * () && noexcept {
      return value_;
    }

    const Type *operator -> () const noexcept {
      return &value_;
    }

    Type *operator -> () noexcept {
      return &value_;
    }

    bool has_value() const noexcept {
      return has_value_;
    }    
  };

  constexpr BoxConstruct kBoxConstruct;
}

#endif