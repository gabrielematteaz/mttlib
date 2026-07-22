#include "file_input_stream.h"

#include <Windows.h>

#include <cassert>
#include <cstring>
#include <new>

namespace mttlib {
  void FileInputStream::Destroy() noexcept {
    CloseHandle(handle_);
    delete[] buffer_;
  }

  Box < FileInputStream > FileInputStream::Construct(const char *path)
      noexcept {
    assert(path != nullptr);

    char *buffer = new(std::nothrow) char[kBufferSize];

    if (buffer == nullptr) {
      return { };
    }

    HANDLE handle = CreateFileA(path, GENERIC_READ, 0,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
      delete[] buffer;

      return { };
    }

    return Box < FileInputStream > (kBoxConstruct, handle, buffer);
  }

  Box < FileInputStream > FileInputStream::Construct(const wchar_t *path)
      noexcept {
    assert(path != nullptr);

    char *buffer = new(std::nothrow) char[kBufferSize];

    if (buffer == nullptr) {
      return { };
    }

    HANDLE handle = CreateFileW(path, GENERIC_READ, 0,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
      delete[] buffer;

      return { };
    }

    return Box < FileInputStream > (kBoxConstruct, handle, buffer);
  }

  FileInputStream::FileInputStream(FileInputStream &&other) noexcept {
    handle_ = other.handle_;
    handle_offset_ = other.handle_offset_;
    buffer_ = other.buffer_;
    buffer_offset_ = other.buffer_offset_;
    valid_count_ = other.valid_count_;
    other.handle_ = INVALID_HANDLE_VALUE;
    other.buffer_ = nullptr;
  }

  FileInputStream &FileInputStream::operator = (FileInputStream &&other) noexcept {
    if (this != &other) {
      Destroy();
      handle_ = other.handle_;
      handle_offset_ = other.handle_offset_;
      buffer_ = other.buffer_;
      buffer_offset_ = other.buffer_offset_;
      valid_count_ = other.valid_count_;
      other.handle_ = INVALID_HANDLE_VALUE;
      other.buffer_ = nullptr;
    }

    return *this;
  }

  int FileInputStream::Read(char *buffer, int count) noexcept {
    assert(buffer != nullptr && count >= 0);

    if (count <= valid_count_) {
      std::memcpy(buffer, buffer_ + buffer_offset_, count);
      buffer_offset_ += count;
      valid_count_ -= count;

      return count;
    }

    int cumulative_count = valid_count_;

    std::memcpy(buffer, buffer_ + buffer_offset_, valid_count_);
    buffer_offset_ = 0;
    valid_count_ = 0;

    while (true) {
      DWORD bytes_read;

      if (count <= cumulative_count + kBufferSize) {
        if (ReadFile(handle_, buffer_, kBufferSize, &bytes_read, NULL) == 0 ||
            bytes_read == 0) {
          return cumulative_count;
        }

        handle_offset_ += bytes_read;

        if (count <= static_cast < int > (cumulative_count + bytes_read)) {
          int remainder = count - cumulative_count;

          std::memcpy(buffer + cumulative_count, buffer_, remainder);
          buffer_offset_ = remainder;
          valid_count_ = bytes_read - remainder;

          return count;
        }
        else { // not enough data
          std::memcpy(buffer + cumulative_count, buffer_, bytes_read);
          cumulative_count += bytes_read;
        }
      }
      else { // direct copy to destination buffer
        if (ReadFile(handle_, buffer + cumulative_count, kBufferSize,
            &bytes_read, NULL) == 0 || bytes_read == 0) {
          return cumulative_count;
        }

        handle_offset_ += bytes_read;
        cumulative_count += bytes_read;
      }
    }
  }

  bool FileInputStream::Seek(std::int64_t offset, SeekPosition position) noexcept {
    std::int64_t real_offset;

    if (position == SeekPosition::kBeginning) {
      real_offset = 0;
    }
    else if (position == SeekPosition::kEnd) {
      LARGE_INTEGER file_size;

      if (GetFileSizeEx(handle_, &file_size) == 0) {
        return false;
      }

      real_offset = file_size.QuadPart;
    }
    else {
      real_offset = handle_offset_;
    }

    real_offset += offset;

    std::int64_t first_offset = handle_offset_ - valid_count_ - buffer_offset_;

    if (real_offset >= first_offset && real_offset <= handle_offset_) {
      buffer_offset_ = static_cast < int > (real_offset - first_offset);
      valid_count_ = static_cast < int > (handle_offset_ - real_offset);
    }
    else {
      LARGE_INTEGER pointer{ .QuadPart = real_offset };

      if (SetFilePointerEx(handle_, pointer, NULL, NULL) == 0) {
        return false;
      }

      handle_offset_ = real_offset;
      buffer_offset_ = 0;
      valid_count_ = 0;
    }

    return true;
  }
}