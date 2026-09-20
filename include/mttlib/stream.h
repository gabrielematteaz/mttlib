#ifndef MTTLIB_INCLUDE_STREAM_H_
#define MTTLIB_INCLUDE_STREAM_H_

#include <cstdint>

namespace mttlib {
  enum class position_t {
    BEGIN,
    CURRENT,
    END
  };

  class read_stream {
  public:
    virtual ~read_stream() = default;
    virtual std::int64_t read(void * buffer, std::int64_t count) noexcept = 0;
    virtual std::int64_t tell() noexcept = 0;
    virtual std::int64_t seek(std::int64_t offset, position_t position) noexcept = 0;
  };

  class write_stream {
  public:
    virtual ~write_stream() = default;
    virtual std::int64_t write(void const* buffer, std::int64_t count) noexcept = 0;
    virtual std::int64_t tell() noexcept = 0;
    virtual std::int64_t seek(std::int64_t offset, position_t position) noexcept = 0;
  };
}

#endif