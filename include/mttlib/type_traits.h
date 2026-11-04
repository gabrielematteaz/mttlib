#ifndef MTTLIB_TYPE_TRAITS_H_
#define MTTLIB_TYPE_TRAITS_H_

namespace mtt {
  template < typename value_type >
  struct remove_reference {
    using result_type = value_type;
  };

  template < typename value_type >
  struct remove_reference < value_type & > {
    using result_type = value_type;
  };

  template < typename value_type >
  struct remove_reference < value_type && > {
    using result_type = value_type;
  };

  template < typename value_type >
  constexpr bool is_lvalue_reference = false;

  template < typename value_type >
  constexpr bool is_lvalue_reference < value_type & > = true;

  template < typename first_type, typename second_type >
  constexpr bool is_same_as = false;

  template < typename value_type >
  constexpr bool is_same_as < value_type, value_type > = true;

  template < typename first_type, typename second_type >
  concept same_as = is_same_as < first_type, second_type >;

  template < typename value_type >
  using remove_reference_type = typename remove_reference < value_type >::result_type;

  template < typename value_type >
  constexpr remove_reference_type < value_type > && move(value_type && value) noexcept {
    return static_cast < remove_reference_type < value_type > && > (value);
  }

  template < typename value_type >
  constexpr value_type && forward(remove_reference_type < value_type > & value) noexcept {
    return static_cast < value_type && > (value);
  }

  template < typename value_type >
  constexpr value_type && forward(remove_reference_type < value_type > && value) noexcept
  requires (is_lvalue_reference < value_type > == false) {
    return static_cast < value_type && > (value);
  }

  template < typename value_type, typename... parameters_type >
  concept constructible_from = requires(parameters_type &&... arguments) {
    { value_type(mtt::forward < parameters_type > (arguments)...) } noexcept;
  };

  template < typename value_type >
  concept copy_assignable = requires(value_type & left, value_type const& right) {
    { left = right } noexcept;
  };

  template < typename value_type >
  concept move_assignable = requires(value_type & left, value_type && right) {
    { left = mtt::move(right) } noexcept;
  };

  template < typename value_type >
  concept copy_constructible = requires(value_type const& value) {
    { value_type(value) } noexcept;
  };

  template < typename value_type >
  concept destructible = requires(value_type const& value) {
    { value.~value_type() } noexcept;
  };

  template < typename value_type >
  concept move_constructible = requires(value_type && value) {
    { value_type(mtt::move(value)) } noexcept;
  };
}

#endif