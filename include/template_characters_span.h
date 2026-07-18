#ifndef MTTLIB_INCLUDE_TEMPLATE_CHARACTERS_SPAN_H_
#define MTTLIB_INCLUDE_TEMPLATE_CHARACTERS_SPAN_H_

#include <cassert>

namespace mttlib {
  template < typename CharacterType >
  constexpr CharacterType kNullTerminator = 0x00;

  template < >
  constexpr char kNullTerminator < char > = '\0';

  template < >
  constexpr wchar_t kNullTerminator < wchar_t > = L'\0';

  template < typename CharacterType >
  // assert(c_string != nullptr)
  int CStringLength(const CharacterType *c_string) noexcept {
    assert(c_string != nullptr);

    const CharacterType *current = c_string;

    while (*current != kNullTerminator < CharacterType >) {
      ++current;
    }

    return static_cast < int > (current - c_string);
  }

  template < typename CharacterType >
  class TemplateCharactersSpan {
    static constexpr CharacterType kFallback[1]{ kNullTerminator < CharacterType > };

    const CharacterType *pointer_;
    int size_;

  public:
    TemplateCharactersSpan() noexcept {
      pointer_ = kFallback;
      size_ = 0;
    }

    // assert(c_string != nullptr)
    explicit TemplateCharactersSpan(const CharacterType *c_string) noexcept {
      assert(c_string != nullptr);

      pointer_ = c_string;
      size_ = CStringLength(c_string);
    }

    // assert(characters != nullptr && count >= 0)
    TemplateCharactersSpan(const CharacterType *characters, int count) noexcept {
      assert(characters != nullptr && count >= 0);

      pointer_ = characters;
      size_ = count;
    }

    const CharacterType *pointer() const noexcept {
      return pointer_;
    }

    int size() const noexcept {
      return size_;
    }
  };

  using CharactersSpan = TemplateCharactersSpan < char >;
  using WideCharactersSpan = TemplateCharactersSpan < wchar_t >;
}

#endif