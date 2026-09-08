#ifndef MTTLIB_INCLUDE_UTILITIES_H_
#define MTTLIB_INCLUDE_UTILITIES_H_

#include "box.h"
#include "template_character_span.h"
#include "template_string.h"

namespace mttlib {
  Box < WideString > UTF8ToUTF16(CharacterSpan character_span) noexcept;
  Box < String > UTF16ToUTF8(WideCharacterSpan character_span) noexcept;
}

#endif