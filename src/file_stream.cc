#include "file_stream.h"

#include <Windows.h>

#include <cstring>
#include <limits>
#include <new>

namespace mttlib {
  Box < FileStream > FileStream::Construct(wchar_t const* path, bool shared) noexcept {
    HANDLE handle = CreateFileW(path, GENERIC_READ, shared ? FILE_SHARE_READ : 0, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
      return { };
    }

    char * buffer = static_cast < char * > (::operator new(kBufferSize * sizeof(*buffer)));

    if (buffer == nullptr) {
      CloseHandle(handle);
      return { };
    }

    return FileStream(handle, buffer);
  }

  Box < FileStream > FileStream::Construct(char const* path, bool shared) noexcept {
    HANDLE handle = CreateFileA(path, GENERIC_READ, shared ? FILE_SHARE_READ : 0, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
      return { };
    }

    char * buffer = static_cast < char * > (::operator new(kBufferSize * sizeof(*buffer)));

    if (buffer == nullptr) {
      CloseHandle(handle);
      return { };
    }

    return FileStream(handle, buffer);
  }

  FileStream::FileStream() noexcept {
    handle_ = INVALID_HANDLE_VALUE;
    handle_offset_ = 0;
    buffer_ = nullptr;
    buffer_offset_ = 0;
    valid_count_ = 0;
  }

  FileStream::FileStream(FileStream &&other) noexcept {
    handle_ = other.handle_;
    handle_offset_ = other.handle_offset_;
    buffer_ = other.buffer_;
    buffer_offset_ = other.buffer_offset_;
    valid_count_ = other.valid_count_;
    other.handle_ = INVALID_HANDLE_VALUE;
    other.buffer_ = nullptr;
  }

  FileStream & FileStream::operator = (FileStream && other) noexcept {
    if (this == &other) {
      return *this;
    }

    Destroy();
    handle_ = other.handle_;
    handle_offset_ = other.handle_offset_;
    buffer_ = other.buffer_;
    buffer_offset_ = other.buffer_offset_;
    valid_count_ = other.valid_count_;
    other.handle_ = INVALID_HANDLE_VALUE;
    other.buffer_ = nullptr;

    return *this;
  }

  std::int64_t FileStream::Read(void * buffer, std::int64_t count) noexcept {
#pragma push_macro("max")
#undef max
    if (count > std::numeric_limits < int >::max()) {
      return -1;
    }
#pragma pop_macro("max")

    int safe_count = static_cast < int > (count);

    if (safe_count <= valid_count_) {
      std::memcpy(buffer, buffer_ + buffer_offset_, safe_count);
      buffer_offset_ = buffer_offset_ + safe_count;
      valid_count_ = valid_count_ - safe_count;

      return safe_count;
    }

    int cumulative_count = valid_count_;

    std::memcpy(buffer, buffer_ + buffer_offset_, valid_count_);
    buffer_offset_ = 0; // fail-safe
    valid_count_ = 0;

    char * char_buffer = static_cast < char * > (buffer);
    int to_read = (safe_count - cumulative_count) / kBufferSize * kBufferSize;
    DWORD bytes_read;

    if (ReadFile(handle_, char_buffer + cumulative_count, to_read, &bytes_read, NULL) == FALSE) {
      return cumulative_count;
    }

    handle_offset_ = handle_offset_ + bytes_read;
    cumulative_count = cumulative_count + bytes_read;

    if (static_cast < int > (bytes_read) != to_read) {
      return cumulative_count;
    }

    if (ReadFile(handle_, buffer_, kBufferSize, &bytes_read, NULL) == FALSE) {
      return cumulative_count;
    }

    handle_offset_ = handle_offset_ + bytes_read;

    std::int64_t final_count = cumulative_count + bytes_read;

    if (final_count < safe_count) {
      std::memcpy(char_buffer + cumulative_count, buffer_, bytes_read);

      return final_count;
    }
    else {
      int to_write = safe_count - cumulative_count;

      std::memcpy(char_buffer + cumulative_count, buffer_, to_write);
      buffer_offset_ = to_write;
      valid_count_ = bytes_read - to_write;

      return safe_count;
    }
  }

  std::int64_t FileStream::Seek(std::int64_t offset, SeekPosition position) noexcept {
    std::int64_t absolute = 0;

    if (position == SeekPosition::kCurrent) {
      absolute = handle_offset_ - valid_count_;
    }
    else if (position == SeekPosition::kEnd) {
      LARGE_INTEGER size;

      if (GetFileSizeEx(handle_, &size) == 0) {
        return -1;
      }

      absolute = size.QuadPart;
    }

    absolute = absolute + offset;

    std::int64_t first = handle_offset_ - valid_count_ - buffer_offset_;
    std::int64_t last = handle_offset_;

    if (absolute < first || absolute > last) {
      LARGE_INTEGER pointer = { .QuadPart = absolute };

      if (SetFilePointerEx(handle_, pointer, NULL, FILE_BEGIN) == 0) {
        return -1;
      }

      buffer_offset_ = 0;
      valid_count_ = 0;
    }
    else {
      buffer_offset_ = static_cast < int > (absolute - first);
      valid_count_ = static_cast < int > (last - absolute);
    }

    return absolute;
  }

  void FileStream::Destroy() noexcept {
    CloseHandle(handle_);
    ::operator delete(buffer_);
  }

  Box < WriteFileStream > WriteFileStream::Construct(wchar_t const* path, bool overwrite, bool shared) noexcept {
    HANDLE handle = CreateFileW(path, GENERIC_WRITE, shared ? FILE_SHARE_WRITE : 0, NULL,
        overwrite ? CREATE_ALWAYS : CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
      return { };
    }

    return WriteFileStream(handle);
  }

  Box < WriteFileStream > WriteFileStream::Construct(char const* path, bool overwrite, bool shared) noexcept {
    HANDLE handle = CreateFileA(path, GENERIC_WRITE, shared ? FILE_SHARE_WRITE : 0, NULL,
        overwrite ? CREATE_ALWAYS : CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
      return { };
    }

    return WriteFileStream(handle);
  }

  WriteFileStream::WriteFileStream() noexcept {
    handle_ = INVALID_HANDLE_VALUE;
    handle_offset_ = 0;
  }

  WriteFileStream::WriteFileStream(WriteFileStream && other) noexcept {
    handle_ = other.handle_;
    handle_offset_ = other.handle_offset_;
    other.handle_ = INVALID_HANDLE_VALUE;
  }

  WriteFileStream & WriteFileStream::operator = (WriteFileStream && other) noexcept {
    if (this == &other) {
      return *this;
    }

    Destroy();
    handle_ = other.handle_;
    handle_offset_ = other.handle_offset_;
    other.handle_ = INVALID_HANDLE_VALUE;

    return *this;
  }

  std::int64_t WriteFileStream::Write(void *buffer, std::int64_t count) noexcept {
#pragma push_macro("max")
#undef max
    if (count > std::numeric_limits < int >::max()) {
      return -1;
    }
#pragma pop_macro("max")

    DWORD bytes_written;

    if (WriteFile(handle_, buffer, static_cast < int > (count), &bytes_written, NULL) == FALSE) {
      return -1;
    }

    handle_offset_ = handle_offset_ + bytes_written;

    return bytes_written;
  }

  std::int64_t WriteFileStream::Seek(std::int64_t offset, SeekPosition position) noexcept {
    DWORD move_method = FILE_BEGIN;

    if (position == SeekPosition::kCurrent) {
      move_method = FILE_CURRENT;
    }
    else if (position == SeekPosition::kEnd) {
      move_method = FILE_END;
    }

    LARGE_INTEGER requested = { .QuadPart = offset };
    LARGE_INTEGER resulting;

    if (SetFilePointerEx(handle_, requested, &resulting, FILE_BEGIN) == 0) {
      return -1;
    }

    handle_offset_ = resulting.QuadPart;

    return resulting.QuadPart;
  }

  void WriteFileStream::Destroy() noexcept {
    CloseHandle(handle_);
  }
}