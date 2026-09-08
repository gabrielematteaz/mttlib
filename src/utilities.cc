#include "utilities.h"

#include <Windows.h>

namespace mttlib {
  Box < WideString > UTF8ToUTF16(CharacterSpan character_span) noexcept {
    if (character_span.empty()) {
      return { };
    }

    int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, character_span.pointer(),
        character_span.size(), NULL, 0);

    if (length == 0) {
      return { };
    }

    Box < WideString > result = WideString::Construct(L'\0', length);

    if (result.has_value() == false || MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
        character_span.pointer(), character_span.size(), result->buffer(), result->size()) == 0) {
      return { };
    }

    return result;
  }

  Box < String > UTF16ToUTF8(WideCharacterSpan character_span) noexcept {
    if (character_span.empty()) {
      return { };
    }

    int length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, character_span.pointer(),
        character_span.size(), NULL, 0, NULL, NULL);

    if (length == 0) {
      return { };
    }

    Box < String > result = String::Construct('\0', length);

    if (result.has_value() == false || WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
        character_span.pointer(), character_span.size(), result->buffer(), result->size(), NULL, NULL) == 0) {
      return { };
    }

    return result;
  }
}