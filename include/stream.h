#ifndef MTTLIB_INCLUDE_STREAM_H_
#define MTTLIB_INCLUDE_STREAM_H_

#include <cstdint>

namespace mttlib {
  enum class SeekPosition {
    kBegin,
    kCurrent,
    kEnd
  };

  class Stream {
  public:
    virtual ~Stream() = default;
    virtual std::int64_t Read(void * buffer, std::int64_t count) noexcept = 0;
    virtual std::int64_t Tell() noexcept = 0;
    virtual std::int64_t Seek(std::int64_t offset, SeekPosition position) noexcept = 0;
  };
}

#endif