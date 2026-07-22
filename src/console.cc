#include "console.h"

#include <Windows.h>

namespace mttlib {
  Box < Console > Console::Construct() noexcept {
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);

    if (input == INVALID_HANDLE_VALUE || input == NULL) {
      return { };
    }

    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);

    if (output == INVALID_HANDLE_VALUE || output == NULL) {
      return { };
    }

    return Box < Console > (kBoxConstruct, input, output);
  }

  ConsoleResult Console::ReadLine(String &string) noexcept {
    string.Clear();

    char buffer[kBufferSize];

    while (true) {
      DWORD bytes_read;

      if (ReadConsoleA(input_, buffer, kBufferSize, &bytes_read, NULL) == 0) {
        return ConsoleResult::kSystemFailure;
      }

      if (bytes_read == 0) {
        return ConsoleResult::kAborted;
      }

      int ignored = 0;

      if (buffer[bytes_read - 1] == '\r') {
        ++ignored;
      }
      else if (buffer[bytes_read - 1] == '\n') {
        ++ignored;

        if (bytes_read > 1 && buffer[bytes_read - 2] == '\r') {
          ++ignored;
        }
      }

      if (string.Append(buffer, bytes_read - ignored) == false) {
        return ConsoleResult::kMemoryAllocationFailure;
      }

      if (ignored != 0) {
        return ConsoleResult::kSuccess;
      }
    }
  }

  ConsoleResult Console::ReadLine(WideString &string) noexcept {
    string.Clear();

    wchar_t buffer[kBufferSize];

    while (true) {
      DWORD bytes_read;

      if (ReadConsoleW(input_, buffer, kBufferSize, &bytes_read, NULL) == 0) {
        return ConsoleResult::kSystemFailure;
      }

      if (bytes_read == 0) {
        return ConsoleResult::kAborted;
      }

      int ignored = 0;

      if (buffer[bytes_read - 1] == '\r') {
        ++ignored;
      }
      else if (buffer[bytes_read - 1] == '\n') {
        ++ignored;

        if (bytes_read > 1 && buffer[bytes_read - 2] == '\r') {
          ++ignored;
        }
      }

      if (string.Append(buffer, bytes_read - ignored) == false) {
        return ConsoleResult::kMemoryAllocationFailure;
      }

      if (ignored != 0) {
        return ConsoleResult::kSuccess;
      }
    }
  }

  int Console::Write(CharactersSpan characters_span, bool append_line_feed)
      const noexcept {
    DWORD bytes_written;

    if (WriteConsoleA(output_, characters_span.pointer(), characters_span.size(),
        &bytes_written, NULL) == 0) {
      return -1;
    }

    if (append_line_feed && WriteConsoleA(output_, kLineFeed, 1, NULL, NULL) != 0) {
      ++bytes_written;
    }

    return bytes_written;
  }

  int Console::Write(WideCharactersSpan characters_span, bool append_line_feed)
      const noexcept {
    DWORD bytes_written;

    if (WriteConsoleW(output_, characters_span.pointer(), characters_span.size(),
        &bytes_written, NULL) == 0) {
      return -1;
    }

    if (append_line_feed && WriteConsoleW(output_, kWideLineFeed, 1, NULL, NULL) != 0) {
      ++bytes_written;
    }

    return bytes_written;
  }
}