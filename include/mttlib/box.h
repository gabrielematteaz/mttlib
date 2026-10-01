#ifndef MTTLIB_BOX_H_
#define MTTLIB_BOX_H_

#include <memory> // std::addressof, std::ranges::construct_at, std::ranges::destroy_at
#include <new>
#include <type_traits>
#include <utility> // std::forward, std::move

namespace mttlib {
  struct box_construct_t {
    explicit box_construct_t() = default;
  };

  template < typename value_type >
  class box {
    union {
      value_type m_value;
    };

    bool m_has_value;

  public:
    box() noexcept {
      m_has_value = false;
    }

    template < typename... parameters_type >
    explicit box(box_construct_t const&, parameters_type &&... arguments) noexcept
    requires std::is_nothrow_constructible_v < value_type, parameters_type... > {
      std::ranges::construct_at(&m_value, std::forward < parameters_type > (arguments)...);
      m_has_value = true;
    }

    box(box const& other) noexcept
    requires std::is_nothrow_copy_constructible_v < value_type > {
      if (other.m_has_value) {
        std::ranges::construct_at(&m_value, other.value);
      }

      m_has_value = other.m_has_value;
    }

    box(box && other) noexcept
    requires std::is_nothrow_move_constructible_v < value_type > {
      if (other.m_has_value) {
        std::ranges::construct_at(&m_value, std::move(other.value));
      }

      m_has_value = other.m_has_value;
    }

    ~box() {
      if (m_has_value) {
        std::ranges::destroy_at(std::addressof(m_value));
      }
    }

    box & operator = (box const& other) noexcept
    requires
        std::is_nothrow_copy_assignable_v < value_type > &&
        std::is_nothrow_copy_constructible_v < value_type > &&
        std::is_nothrow_destructible_v < value_type > {
      if (other.m_has_value) {
        if (m_has_value) {
          m_value = other.m_value;
        }
        else {
          std::ranges::construct_at(&m_value, other.value);
        }
      }
      else if (m_has_value) {
        std::ranges::destroy_at(std::addressof(m_value));
      }

      m_has_value = other.m_has_value;
    }

    box & operator = (box && other) noexcept
    requires
        std::is_nothrow_move_assignable_v < value_type > &&
        std::is_nothrow_move_constructible_v < value_type > &&
        std::is_nothrow_destructible_v < value_type > {
      if (other.m_has_value) {
        if (m_has_value) {
          m_value = std::move(other.m_value);
        }
        else {
          std::ranges::construct_at(&m_value, std::move(other.value));
        }
      }
      else if (m_has_value) {
        std::ranges::destroy_at(std::addressof(m_value));
      }

      m_has_value = other.m_has_value;
    }

    explicit operator bool() const noexcept {
      return m_has_value;
    }

    value_type const* operator -> () const noexcept {
      return std::addressof(m_value);
    }

    value_type * operator -> () noexcept {
      return std::addressof(m_value);
    }

    value_type const& operator * () const& noexcept {
      return m_value;
    }

    value_type & operator * () & noexcept {
      return m_value;
    }

    value_type const&& operator * () const&& noexcept {
      return std::move(m_value);
    }

    value_type && operator * () && noexcept {
      return std::move(m_value);
    }

    bool has_value() const noexcept {
      return m_has_value;
    }

    value_type const& value() const& noexcept {
      return m_value;
    }

    value_type & value() & noexcept {
      return m_value;
    }

    value_type const&& value() const&& noexcept {
      return std::move(m_value);
    }

    value_type && value() && noexcept {
      return std::move(m_value);
    }
  };

  constexpr box_construct_t BOX_CONSTRUCT;
}

#endif