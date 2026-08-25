#include "file_stream.h"

#include <cassert>
#include <cstring> // std::memcpy
#include <new>

#include <Windows.h>

namespace mttlib {
  Box < FileReadStream > FileReadStream::Construct(wchar_t const* path, bool shared) noexcept {
    HANDLE handle = CreateFileW(path, GENERIC_READ, shared ? FILE_SHARE_READ : 0, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
      return { };
    }

    char * buffer = new(std::nothrow) char[kBufferSize];

    if (buffer == nullptr) {
      CloseHandle(handle);
      return { };
    }

    return Box < FileReadStream > (kBoxConstruct, FileReadStream(handle, buffer));
  }

  FileReadStream::FileReadStream(FileReadStream && other) noexcept {
    handle_ = other.handle_;
    handle_offset_ = other.handle_offset_;
    buffer_ = other.buffer_;
    buffer_offset_ = other.buffer_offset_;
    valid_count_ = other.valid_count_;
    other.handle_ = INVALID_HANDLE_VALUE;
    other.buffer_ = nullptr;
  }

  FileReadStream & FileReadStream::operator = (FileReadStream && other) noexcept {
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

  std::int64_t FileReadStream::Read(void * buffer, int count) noexcept {
    assert(buffer != nullptr && count >= 0);

    if (count <= valid_count_) {
      std::memcpy(buffer, buffer_ + buffer_offset_, count);
      buffer_offset_ = buffer_offset_ + count;
      valid_count_ = valid_count_ - count;

      return count;
    }

    int cumulative_count = valid_count_;

    std::memcpy(buffer, buffer_ + buffer_offset_, valid_count_);
    buffer_offset_ = 0;
    valid_count_ = 0;

    char * char_buffer = static_cast < char * > (buffer);
    int to_read = (count - cumulative_count) / kBufferSize * kBufferSize;
    DWORD bytes_read;
    BOOL result = ReadFile(handle_, char_buffer + cumulative_count, to_read, &bytes_read, NULL);

    if (result == FALSE) {
      return cumulative_count;
    }

    cumulative_count = cumulative_count + bytes_read;
    handle_offset_ = handle_offset_ + bytes_read;

    if (static_cast < int > (bytes_read) != to_read) {
      return cumulative_count;
    }

    if (ReadFile(handle_, buffer_, kBufferSize, &bytes_read, NULL) == FALSE) {
      return cumulative_count;
    }

    handle_offset_ = handle_offset_ + bytes_read;

    int to_copy = count - cumulative_count;

    if (static_cast < int > (bytes_read) < to_copy) {
      std::memcpy(char_buffer + cumulative_count, buffer_, bytes_read);
      cumulative_count = cumulative_count + bytes_read;
    }
    else {
      std::memcpy(char_buffer + cumulative_count, buffer_, to_copy);
      buffer_offset_ = to_copy;
      valid_count_ = bytes_read - to_copy;
      cumulative_count = cumulative_count + to_copy;
    }

    return cumulative_count;
  }

  bool FileReadStream::Seek(std::int64_t offset, SeekPosition position) noexcept {
    assert(position == SeekPosition::kBeginning || position == SeekPosition::kCurrent ||
        position == SeekPosition::kEnd);

    std::int64_t absolute;

    switch (position) {
      case SeekPosition::kBeginning:
        absolute = 0;

        break;
      case SeekPosition::kCurrent:
        absolute = handle_offset_ - valid_count_;

        break;
      case SeekPosition::kEnd: {
        LARGE_INTEGER size;

        if (GetFileSizeEx(handle_, &size) == 0) {
          return false;
        }

        absolute = size.QuadPart;

        break;
      }
    }

    absolute = absolute + offset;

    std::int64_t first = handle_offset_ - valid_count_ - buffer_offset_;
    std::int64_t last = handle_offset_;

    if (absolute >= first && absolute <= last) {
      buffer_offset_ = static_cast < int > (absolute - first);
      valid_count_ = static_cast < int > (last - buffer_offset_);
    }
    else {
      LARGE_INTEGER pointer = { .QuadPart = absolute };

      if (SetFilePointerEx(handle_, pointer, NULL, FILE_BEGIN) == 0) {
        return false;
      }

      handle_offset_ = absolute;
      buffer_offset_ = 0;
      valid_count_ = 0;
    }

    return true;
  }

  void FileReadStream::Destroy() noexcept {
    CloseHandle(handle_);
    delete[] buffer_;
  }
}