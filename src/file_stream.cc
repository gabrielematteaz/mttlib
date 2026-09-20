#include "file_stream.h"

#include <Windows.h>

#include <cstring>
#include <limits>
#include <new>

namespace mttlib {
  box < file_read_stream > file_read_stream::construct(wchar_t const* path, bool shared) noexcept {
    HANDLE handle = CreateFileW(path, GENERIC_READ, shared ? FILE_SHARE_READ : 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
      return { };
    }

    char * buffer = static_cast < char * > (::operator new(BUFFER_SIZE * sizeof(*buffer)));

    if (buffer == nullptr) {
      CloseHandle(handle);
      return { };
    }

    return file_read_stream(handle, buffer);
  }

  box < file_read_stream > file_read_stream::construct(char const* path, bool shared) noexcept {
    HANDLE handle = CreateFileA(path, GENERIC_READ, shared ? FILE_SHARE_READ : 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
      return { };
    }

    char * buffer = static_cast < char * > (::operator new(BUFFER_SIZE * sizeof(*buffer)));

    if (buffer == nullptr) {
      CloseHandle(handle);
      return { };
    }

    return file_read_stream(handle, buffer);
  }

  file_read_stream::file_read_stream() noexcept {
    m_handle = INVALID_HANDLE_VALUE;
    m_handle_offset = 0;
    m_buffer = nullptr;
    m_buffer_offset = 0;
    m_valid_count = 0;
  }

  file_read_stream::file_read_stream(file_read_stream &&other) noexcept {
    m_handle = other.m_handle;
    m_handle_offset = other.m_handle_offset;
    m_buffer = other.m_buffer;
    m_buffer_offset = other.m_buffer_offset;
    m_valid_count = other.m_valid_count;
    other.m_handle = INVALID_HANDLE_VALUE;
    other.m_buffer = nullptr;
  }

  file_read_stream & file_read_stream::operator = (file_read_stream && other) noexcept {
    if (this == &other) {
      return *this;
    }

    destroy();
    m_handle = other.m_handle;
    m_handle_offset = other.m_handle_offset;
    m_buffer = other.m_buffer;
    m_buffer_offset = other.m_buffer_offset;
    m_valid_count = other.m_valid_count;
    other.m_handle = INVALID_HANDLE_VALUE;
    other.m_buffer = nullptr;

    return *this;
  }

  std::int64_t file_read_stream::read(void * buffer, std::int64_t count) noexcept {
#pragma push_macro("max")
#undef max
    if (count > std::numeric_limits < int >::max()) {
      return -1;
    }
#pragma pop_macro("max")

    int safe_count = static_cast < int > (count);

    if (safe_count <= m_valid_count) {
      std::memcpy(buffer, m_buffer + m_buffer_offset, safe_count);
      m_buffer_offset = m_buffer_offset + safe_count;
      m_valid_count = m_valid_count - safe_count;

      return safe_count;
    }

    int cumulative_count = m_valid_count;

    std::memcpy(buffer, m_buffer + m_buffer_offset, m_valid_count);
    m_buffer_offset = 0; // fail-safe
    m_valid_count = 0;

    char * char_buffer = static_cast < char * > (buffer);
    int to_read = (safe_count - cumulative_count) / BUFFER_SIZE * BUFFER_SIZE;
    DWORD bytes_read;

    if (ReadFile(m_handle, char_buffer + cumulative_count, to_read, &bytes_read, NULL) == FALSE) {
      return cumulative_count;
    }

    m_handle_offset = m_handle_offset + bytes_read;
    cumulative_count = cumulative_count + bytes_read;

    if (static_cast < int > (bytes_read) != to_read) {
      return cumulative_count;
    }

    if (ReadFile(m_handle, m_buffer, BUFFER_SIZE, &bytes_read, NULL) == FALSE) {
      return cumulative_count;
    }

    m_handle_offset = m_handle_offset + bytes_read;

    std::int64_t final_count = cumulative_count + bytes_read;

    if (final_count < safe_count) {
      std::memcpy(char_buffer + cumulative_count, m_buffer, bytes_read);

      return final_count;
    }
    else {
      int to_write = safe_count - cumulative_count;

      std::memcpy(char_buffer + cumulative_count, m_buffer, to_write);
      m_buffer_offset = to_write;
      m_valid_count = bytes_read - to_write;

      return safe_count;
    }
  }

  std::int64_t file_read_stream::seek(std::int64_t offset, position_t position) noexcept {
    std::int64_t absolute = 0;

    if (position == position_t::CURRENT) {
      absolute = m_handle_offset - m_valid_count;
    }
    else if (position == position_t::CURRENT) {
      LARGE_INTEGER size;

      if (GetFileSizeEx(m_handle, &size) == 0) {
        return -1;
      }

      absolute = size.QuadPart;
    }

    absolute = absolute + offset;

    std::int64_t first = m_handle_offset - m_valid_count - m_buffer_offset;
    std::int64_t last = m_handle_offset;

    if (absolute < first || absolute > last) {
      LARGE_INTEGER pointer = { .QuadPart = absolute };

      if (SetFilePointerEx(m_handle, pointer, NULL, FILE_BEGIN) == 0) {
        return -1;
      }

      m_buffer_offset = 0;
      m_valid_count = 0;
    }
    else {
      m_buffer_offset = static_cast < int > (absolute - first);
      m_valid_count = static_cast < int > (last - absolute);
    }

    return absolute;
  }

  void file_read_stream::destroy() noexcept {
    CloseHandle(m_handle);
    ::operator delete(m_buffer);
  }

  box < write_file_stream > write_file_stream::construct(wchar_t const* path, bool overwrite, bool shared) noexcept {
    HANDLE handle = CreateFileW(path, GENERIC_WRITE, shared ? FILE_SHARE_WRITE : 0, NULL, overwrite ? CREATE_ALWAYS : CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
      return { };
    }

    return write_file_stream(handle);
  }

  box < write_file_stream > write_file_stream::construct(char const* path, bool overwrite, bool shared) noexcept {
    HANDLE handle = CreateFileA(path, GENERIC_WRITE, shared ? FILE_SHARE_WRITE : 0, NULL, overwrite ? CREATE_ALWAYS : CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
      return { };
    }

    return write_file_stream(handle);
  }

  write_file_stream::write_file_stream() noexcept {
    m_handle = INVALID_HANDLE_VALUE;
    m_handle_offset = 0;
  }

  write_file_stream::write_file_stream(write_file_stream && other) noexcept {
    m_handle = other.m_handle;
    m_handle_offset = other.m_handle_offset;
    other.m_handle = INVALID_HANDLE_VALUE;
  }

  write_file_stream & write_file_stream::operator = (write_file_stream && other) noexcept {
    if (this == &other) {
      return *this;
    }

    destroy();
    m_handle = other.m_handle;
    m_handle_offset = other.m_handle_offset;
    other.m_handle = INVALID_HANDLE_VALUE;

    return *this;
  }

  std::int64_t write_file_stream::write(void const* buffer, std::int64_t count) noexcept {
#pragma push_macro("max")
#undef max
    if (count > std::numeric_limits < int >::max()) {
      return -1;
    }
#pragma pop_macro("max")

    DWORD bytes_written;

    if (WriteFile(m_handle, buffer, static_cast < int > (count), &bytes_written, NULL) == FALSE) {
      return -1;
    }

    m_handle_offset = m_handle_offset + bytes_written;

    return bytes_written;
  }

  std::int64_t write_file_stream::seek(std::int64_t offset, position_t position) noexcept {
    DWORD move_method = FILE_BEGIN;

    if (position == position_t::CURRENT) {
      move_method = FILE_CURRENT;
    }
    else if (position == position_t::CURRENT) {
      move_method = FILE_END;
    }

    LARGE_INTEGER requested = { .QuadPart = offset };
    LARGE_INTEGER resulting;

    if (SetFilePointerEx(m_handle, requested, &resulting, FILE_BEGIN) == 0) {
      return -1;
    }

    m_handle_offset = resulting.QuadPart;

    return resulting.QuadPart;
  }

  void write_file_stream::destroy() noexcept {
    CloseHandle(m_handle);
  }
}