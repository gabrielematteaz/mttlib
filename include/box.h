#ifndef MTTLIB_INCLUDE_BOX_H_
#define MTTLIB_INCLUDE_BOX_H_

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
    Box() noexcept;
    template < typename ... ConstructorParametersType >
    Box(BoxConstruct const&, ConstructorParametersType &&... constructor_arguments) noexcept;
    Box(Box const& other) noexcept;
    Box(Box && other) noexcept;
    ~Box();
    Box & operator = (Box const& other) noexcept;
    Box & operator = (Box && other) noexcept;
    explicit operator bool() const noexcept;
    ValueType const* operator -> () const noexcept;
    ValueType * operator -> () noexcept;
    ValueType const& operator * () const& noexcept;
    ValueType & operator * () & noexcept;
    ValueType const&& operator * () const&& noexcept;
    ValueType && operator * && noexcept;
    bool has_value() const noexcept;
    ValueType const& value() const& noexcept;
    ValueType & value() & noexcept;
    ValueType const&& value() const&& noexcept;
    ValueType && value() && noexcept;
  };

  constexpr BoxConstruct kBoxConstruct;
}

#endif