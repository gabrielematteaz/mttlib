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
      ValueType this_value;
    };

    bool this_has_value;

  public:
    Box() noexcept {
      this_has_value = false;
    }

    template < typename... ConstructorParametersType >
    explicit Box(BoxConstruct const&, ConstructorParametersType &&... constructor_arguments) noexcept {
      static_assert(std::is_nothrow_constructible_v < ValueType, ConstructorParametersType... >);

      new(&this_value) ValueType(std::forward < ConstructorParametersType > (constructor_arguments)...);
      this_has_value = true;
    }

    Box(Box const& other) noexcept {
      static_assert(std::is_nothrow_copy_constructible_v < ValueType >);

      if (other.this_has_value) {
        new(&this_value) ValueType(other.this_value);
      }

      this_has_value = other.this_has_value;
    }

    Box(Box && other) noexcept {
      static_assert(std::is_nothrow_move_constructible_v < ValueType >);

      if (other.this_has_value) {
        new(&this_value) ValueType(std::move(other.this_value));
      }

      this_has_value = other.this_has_value;
      other.this_has_value = false;
    }

    ~Box() {
      static_assert(std::is_nothrow_destructible_v < ValueType >);

      if (this_has_value) {
        this_value.~ValueType();
      }
    }

    Box & operator = (Box const& other) noexcept {
      static_assert(std::is_nothrow_copy_assignable_v < ValueType > && std::is_nothrow_destructible_v < ValueType> &&
          std::is_nothrow_copy_constructible_v < ValueType >);

      if (this != &other) {
        if (this_has_value) {
          if (other.this_has_value) {
            this_value = other.this_value;
          }
          else {
            this_value.~ValueType();
          }
        }
        else if (other.this_has_value) {
          new(&this_value) ValueType(other.this_value);
        }

        this_has_value = other.this_has_value;
      }

      return *this;
    }

    Box & operator = (Box && other) noexcept {
      static_assert(std::is_nothrow_move_assignable_v < ValueType > && std::is_nothrow_destructible_v < ValueType> &&
          std::is_nothrow_move_constructible_v < ValueType >);

      if (this != &other) {
        if (this_has_value) {
          if (other.this_has_value) {
            this_value = std::move(other.this_value);
          }
          else {
            this_value.~ValueType();
          }
        }
        else if (other.this_has_value) {
          new(&this_value) ValueType(std::move(other.this_value));
        }

        this_has_value = other.this_has_value;
        other.this_has_value = false;
      }

      return *this;
    }

    explicit operator bool() const noexcept {
      return this_has_value;
    }

    ValueType const* operator -> () const noexcept {
      return &this_value;
    }

    ValueType * operator -> () noexcept {
      return &this_value;
    }

    ValueType const& operator * () const& noexcept {
      return this_value;
    }

    ValueType & operator * () & noexcept {
      return this_value;
    }

    ValueType const&& operator * () const&& noexcept {
      return this_value;
    }

    ValueType && operator * () && noexcept {
      return this_value;
    }

    bool has_value() const noexcept {
      return this_has_value;
    }

    ValueType const& value() const& noexcept {
      return this_value;
    }

    ValueType & value() & noexcept {
      return this_value;
    }

    ValueType const&& value() const&& noexcept {
      return this_value;
    }

    ValueType && value() && noexcept {
      return this_value;
    }
  };

  constexpr BoxConstruct kBoxConstruct;
}

#endif