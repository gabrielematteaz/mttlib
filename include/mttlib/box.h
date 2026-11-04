#ifndef MTTLIB_BOX_H_
#define MTTLIB_BOX_H_

#include <memory> // std::addressof

#include "type_traits.h"

namespace mtt {
  struct box_construct_tag {
    explicit box_construct_tag() = default;
  };

  template < typename value_type >
  class box {
  public:
    constexpr box() noexcept {
      m_has_value = false;
    }

    constexpr box(box const& other) noexcept
    requires copy_constructible < value_type > {
      if (other.m_has_value) {
        ::new(static_cast < void * > (std::addressof(m_value))) value_type(other.m_value);
      }

      m_has_value = other.m_has_value;
    }

    constexpr box(box && other) noexcept
    requires move_constructible < value_type > {
      if (other.m_has_value) {
        ::new(static_cast < void * > (std::addressof(m_value))) value_type(mtt::move(other.m_value));
      }

      m_has_value = other.m_has_value;
    }

    template < typename... parameters_type >
    constexpr explicit box(box_construct_tag const&, parameters_type &&... arguments) noexcept
    requires constructible_from < value_type, parameters_type... > {
      ::new(static_cast < void * > (std::addressof(m_value))) value_type(mtt::forward < parameters_type > (arguments)...);
      m_has_value = true;
    }

    constexpr ~box()
    requires destructible < value_type > {
      if (m_has_value) {
        m_value.~value_type();
      }
    }

    constexpr box & operator = (box const& other) noexcept
    requires copy_assignable < value_type > && copy_constructible < value_type > && destructible < value_type > {
      if (this != &other) {
        if (other.m_has_value) {
          if (m_has_value) {
            m_value = other.m_value;
          }
          else {
            ::new(static_cast < void * > (std::addressof(m_value))) value_type(other.m_value);
          }
        }
        else {
          if (m_has_value) {
            m_value.~value_type();
          }
        }

        m_has_value = other.m_has_value;
      }

      return *this;
    }

    constexpr box & operator = (box const& other) noexcept
    requires move_assignable < value_type > && move_constructible < value_type > && destructible < value_type > {
      if (this != &other) {
        if (other.m_has_value) {
          if (m_has_value) {
            m_value = mtt::move(other.m_value);
          }
          else {
            ::new(static_cast < void * > (std::addressof(m_value))) value_type(mtt::move(other.m_value));
          }
        }
        else {
          if (m_has_value) {
            m_value.~value_type();
          }
        }

        m_has_value = other.m_has_value;
      }

      return *this;
    }

    /////

    constexpr explicit operator bool() const noexcept {
      return m_has_value;
    }

    constexpr value_type const* operator -> () const noexcept {
      return std::addressof(m_value);
    }

    constexpr value_type * operator -> () noexcept {
      return std::addressof(m_value);
    }

    constexpr value_type const& operator * () const& noexcept {
      return m_value;
    }

    constexpr value_type & operator * () & noexcept {
      return m_value;
    }

    constexpr value_type const&& operator * () const&& noexcept {
      return mtt::move(m_value);
    }

    constexpr value_type && operator * () && noexcept {
      return mtt::move(m_value);
    }

    /////

    constexpr bool has_value() const noexcept {
      return m_has_value;
    }

    constexpr value_type const& value() const& noexcept {
      return m_value;
    }

    constexpr value_type & value() & noexcept {
      return m_value;
    }

    constexpr value_type const&& value() const&& noexcept {
      return mtt::move(m_value);
    }

    constexpr value_type && value() && noexcept {
      return mtt::move(m_value);
    }

  private:
    union { value_type m_value; };
    bool m_has_value;
  };

  constexpr box_construct_tag BOX_CONSTRUCT;

  template < typename value_type, typename... parameters_type >
  concept has_construct = requires(parameters_type &&... arguments) {
    { value_type::construct(mtt::forward < parameters_type > (arguments)...) } noexcept -> same_as < box < value_type > >;
  };

  template < typename value_type, typename... parameters_type >
  constexpr box < value_type > construct(parameters_type &&... arguments) noexcept
  requires has_construct < value_type, parameters_type... > || constructible_from < value_type, parameters_type... > {
    if constexpr (has_construct < value_type, parameters_type... >) {
      return value_type::construct(mtt::forward < parameters_type > (arguments)...);
    }
    else {
      return box < value_type > (BOX_CONSTRUCT, mtt::forward < parameters_type > (arguments)...);
    }
  }
}

#endif