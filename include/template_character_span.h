#ifndef MTTLIB_INCLUDE_TEMPLATE_CHARACTERS_SPAN_H_
#define MTTLIB_INCLUDE_TEMPLATE_CHARACTERS_SPAN_H_

namespace mttlib {
  template < typename CharacterType >
  constexpr CharacterType kNullTerminator = 0x00;

  template < >
  constexpr char kNullTerminator < char > = '\0';

  template < >
  constexpr wchar_t kNullTerminator < wchar_t > = L'\0';

  template < typename CharacterType >
  inline int CStringLength(CharacterType const* c_string) noexcept {
    CharacterType const* current = c_string;

    while (*current != kNullTerminator < CharacterType >) {
      ++current;
    }

    return static_cast < int > (current - c_string);
  }

  template < typename CharacterType >
  class TemplateCharacterSpan {
    static constexpr CharacterType kFallback[1]{ kNullTerminator < CharacterType > };

    CharacterType const* pointer_;
    int size_;

  public:
    TemplateCharacterSpan() noexcept {
      pointer_ = kFallback;
      size_ = 0;
    }

    TemplateCharacterSpan(CharacterType const* c_string) noexcept {
      pointer_ = c_string;
      size_ = CStringLength(c_string);
    }

    TemplateCharacterSpan(CharacterType const* characters, int count) noexcept {
      pointer_ = characters;
      size_ = count;
    }

    CharacterType const* pointer() const noexcept {
      return pointer_;
    }

    int size() const noexcept {
      return size_;
    }
  };

  using CharacterSpan = TemplateCharacterSpan < char >;
  using WideCharacterSpan = TemplateCharacterSpan < wchar_t >;
}

#endif