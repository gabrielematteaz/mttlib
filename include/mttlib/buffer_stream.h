#ifndef MTTLIB_INCLUDE_BUFFER_STREAM_H_
#define MTTLIB_INCLUDE_BUFFER_STREAM_H_

#include <cstdint>
#include <cstring>

#include "stream.h"

namespace mttlib {
  class buffer_read_stream : public read_stream {
    char * m_buffer;
    std::int64_t m_offset;
    std::int64_t m_valid_count;

  public:
    buffer_read_stream() noexcept {
      m_buffer = nullptr;
      m_offset = 0;
      m_valid_count = 0;
    }

    buffer_read_stream(void * buffer, std::int64_t valid_count) noexcept {
      m_buffer = static_cast < char * > (buffer);
      m_offset = 0;
      m_valid_count = valid_count;
    }

    std::int64_t read(void * buffer, std::int64_t count) noexcept override {
      if (count > m_valid_count) {
        count = m_valid_count;
      }

      std::memcpy(buffer, m_buffer + m_offset, count);
      m_offset = m_offset + count;
      m_valid_count = m_valid_count - count;

      return count;
    }

    std::int64_t tell() noexcept override {
      return m_offset;
    }

    std::int64_t seek(std::int64_t offset, position_t position) noexcept override {
      std::int64_t absolute = 0;
      std::int64_t last = m_offset + m_valid_count;

      if (position == position_t::CURRENT) {
        absolute = m_offset;
      }
      else if (position == position_t::END) {
        absolute = last;
      }

      absolute = absolute + offset;

      if (absolute < 0 || absolute > last) {
        return -1;
      }

      m_offset = absolute;
      m_valid_count = last - absolute;

      return absolute;
    }
  };
}

#endif