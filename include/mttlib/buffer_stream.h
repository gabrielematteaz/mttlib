#ifndef MTTLIB_INCLUDE_BUFFER_STREAM_H_
#define MTTLIB_INCLUDE_BUFFER_STREAM_H_

#include <cstdint>
#include <cstring>

#include "stream.h"

namespace mttlib {
  class BufferStream : public Stream {
    char * buffer_;
    std::int64_t offset_;
    std::int64_t valid_count_;

  public:
    BufferStream() noexcept {
      buffer_ = nullptr;
      offset_ = 0;
      valid_count_ = 0;
    }

    BufferStream(void * buffer, std::int64_t valid_count) noexcept {
      buffer_ = static_cast < char * > (buffer);
      offset_ = 0;
      valid_count_ = valid_count;
    }

    std::int64_t Read(void * buffer, std::int64_t count) noexcept override {
      if (count > valid_count_) {
        count = valid_count_;
      }

      std::memcpy(buffer, buffer_ + offset_, count);
      offset_ = offset_ + count;
      valid_count_ = valid_count_ - count;

      return count;
    }

    std::int64_t Tell() noexcept override {
      return offset_;
    }

    std::int64_t Seek(std::int64_t offset, SeekPosition position) noexcept override {
      std::int64_t absolute = 0;
      std::int64_t last = offset_ + valid_count_;

      if (position == SeekPosition::kCurrent) {
        absolute = offset_;
      }
      else if (position == SeekPosition::kEnd) {
        absolute = last;
      }

      absolute = absolute + offset;

      if (absolute < 0 || absolute > last) {
        return -1;
      }

      offset_ = absolute;
      valid_count_ = last - absolute;

      return absolute;
    }
  };
}

#endif