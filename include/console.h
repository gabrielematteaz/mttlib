#ifndef MTTLIB_INCLUDE_CONSOLE_H_
#define MTTLIB_INCLUDE_CONSOLE_H_

#include <cassert>
#include <optional>

#include "template_string.h"

namespace mttlib {
  enum class ConsoleResult {
    kSuccess,
    kSystemFailure,
    kAborted,
    kMemoryAllocationFailure
  };

  class Console {
    static constexpr int kBufferSize = 128;
    static constexpr char kLineFeed[1]{ '\n' };
    static constexpr wchar_t kWideLineFeed[1]{ L'\n' };

    void *input_;
    void *output_;

    Console(void *input, void *output) noexcept {
      input_ = input;
      output_ = output;
    }

  public:
    static std::optional < Console > Construct() noexcept;
    ConsoleResult ReadLine(String &string) noexcept;
    ConsoleResult ReadLine(WideString &string) noexcept;
    int Write(CharactersSpan characters_span, bool append_line_feed) const noexcept;
    int Write(WideCharactersSpan characters_span, bool append_line_feed) const noexcept;

    // assert(c_string != nullptr)
    int Write(const char *c_string, bool append_line_feed) const noexcept {
      assert(c_string != nullptr);

      return Write(CharactersSpan(c_string), append_line_feed);
    }

    // assert(c_string != nullptr)
    int Write(const wchar_t *c_string, bool append_line_feed) const noexcept {
      assert(c_string != nullptr);

      return Write(WideCharactersSpan(c_string), append_line_feed);
    }

    // assert(characters != nullptr && count >= 0)
    int Write(const char *characters, int count, bool append_line_feed) const noexcept {
      assert(characters != nullptr && count >= 0);

      return Write(CharactersSpan(characters, count), append_line_feed);
    }

    // assert(characters != nullptr && count >= 0)
    int Write(const wchar_t *characters, int count, bool append_line_feed) const noexcept {
      assert(characters != nullptr && count >= 0);

      return Write(WideCharactersSpan(characters, count), append_line_feed);
    }
  };
}

#endif