#ifndef MTTLIB_INCLUDE_CONSOLE_H_
#define MTTLIB_INCLUDE_CONSOLE_H_

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

    void *input_;
    void *output_;

    Console(void *input, void *output) noexcept {
      input_ = input;
      output_ = output;
    }

  public:
    static std::optional < Console > Construct() noexcept;
    ConsoleResult ReadLine(String &string) noexcept;
    int Write(CharactersSpan characters_span, bool append_line_feed) const noexcept;
  };
}

#endif