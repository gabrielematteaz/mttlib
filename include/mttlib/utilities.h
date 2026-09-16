#ifndef MTTLIB_INCLUDE_UTILITIES_H_
#define MTTLIB_INCLUDE_UTILITIES_H_

#include <type_traits>
#include <utility> // std::move

namespace mttlib {
  // designed to be safely used with empty ranges (especially the "nullptr + 0" case)

  template < typename value_type >
  inline void copy_construct_uninitialized(value_type * source, int offset, int count,
      value_type const& value) noexcept
  requires std::is_nothrow_copy_constructible_v < value_type > {
    if (count == 0) {
      return 0;
    }

    value_type * current = source + offset;

    do {
      ::new(static_cast < void * > (current)) value_type(value);
      ++current;
      --count;
    } while (count != 0);
  }

  template < typename value_type >
  inline void default_construct_uninitialized(value_type * memory, int offset, int count) noexcept
  requires std::is_nothrow_default_constructible_v < value_type > {
    if (count == 0) {
      return;
    }

    value_type * current = memory + offset;

    do {
      ::new(static_cast < void * > (current)) value_type();
      ++current;
      --count;
    } while (count != 0);
  }

  template < typename value_type >
  inline void destroy(value_type * memory, int offset, int count) noexcept
  requires std::is_nothrow_destructible_v < value_type > {
    if (count == 0) {
      return;
    }

    value_type * current = memory + offset;

    do {
      current->~value_type();
      ++current;
      --count;
    } while (count != 0);
  }

  template < typename value_type >
  inline void move_construct_uninitialized(value_type * source, int offset, int count,
      value_type * destination) noexcept
  requires std::is_nothrow_move_constructible_v < value_type > {
    if (count == 0) {
      return;
    }

    value_type * current = source + offset;

    do {
      ::new(static_cast < void * > (destination)) value_type(std::move(*current));
      ++current;
      --count;
      ++destination;
    } while (count != 0);
  }
}

#endif