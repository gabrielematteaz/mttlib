#ifndef MTTLIB_INCLUDE_FILE_STREAM_H_
#define MTTLIB_INCLUDE_FILE_STREAM_H_

#include <cstdint>

#include "box.h"
#include "stream.h"

namespace mttlib {
  class file_read_stream : public read_stream {
    static constexpr int BUFFER_SIZE = 4096;

    void * m_handle;
    std::int64_t m_handle_offset;
    char * m_buffer;
    int m_buffer_offset;
    int m_valid_count;

  public:
    static box < file_read_stream > construct(wchar_t const* path, bool shared) noexcept;
    static box < file_read_stream > construct(char const* path, bool shared) noexcept;
    file_read_stream() noexcept;
    file_read_stream(file_read_stream const&) = delete;
    file_read_stream(file_read_stream && other) noexcept;
    file_read_stream & operator = (file_read_stream const&) = delete;
    file_read_stream & operator = (file_read_stream && other) noexcept;
    std::int64_t read(void * buffer, std::int64_t count) noexcept override;
    std::int64_t seek(std::int64_t offset, position_t position) noexcept override;

    ~file_read_stream() {
      destroy();
    }

    std::int64_t tell() noexcept override {
      return m_handle_offset - m_valid_count;
    }

  private:
    void destroy() noexcept;

    file_read_stream(void * handle, char * buffer) noexcept {
      m_handle = handle;
      m_handle_offset = 0;
      m_buffer = buffer;
      m_buffer_offset = 0;
      m_valid_count = 0;
    }
  };

  class write_file_stream : public write_stream {
    void * m_handle;
    std::int64_t m_handle_offset;

  public:
    static box < write_file_stream > construct(wchar_t const* path, bool overwrite, bool shared) noexcept;
    static box < write_file_stream > construct(char const* path, bool overwrite, bool shared) noexcept;
    write_file_stream() noexcept;
    write_file_stream(write_file_stream const&) = delete;
    write_file_stream(write_file_stream && other) noexcept;
    write_file_stream & operator = (write_file_stream const&) = delete;
    write_file_stream & operator = (write_file_stream && other) noexcept;
    std::int64_t write(void const* buffer, std::int64_t count) noexcept override;
    std::int64_t seek(std::int64_t offset, position_t position) noexcept override;

    ~write_file_stream() {
      destroy();
    }

    std::int64_t tell() noexcept override {
      return m_handle_offset;
    }

  private:
    void destroy() noexcept;

    write_file_stream(void * handle) noexcept {
      m_handle = handle;
      m_handle_offset = 0;
    }
  };
}

#endif