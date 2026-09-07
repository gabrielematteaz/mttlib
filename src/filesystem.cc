#include "mttlib\filesystem.h"

#include <Windows.h>

namespace mttlib {
  bool CreateDirectories(char const* path) noexcept {
    return CreateDirectoryA(path, NULL) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
  }
}