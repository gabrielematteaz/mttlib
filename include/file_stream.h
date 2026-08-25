#ifndef MTTLIB_INCLUDE_FILE_STREAM_H_
#define MTTLIB_INCLUDE_FILE_STREAM_H_

#include <cstdint> // std::int64_t

#include "box.h"

namespace mttlib {
  enum class SeekPosition {
    kBeginning,
    kCurrent,
    kEnd
  };

  class FileReadStream {
    static constexpr int kBufferSize = 4096;

    void * handle_;
    std::int64_t handle_offset_;
    char * buffer_;
    int buffer_offset_;
    int valid_count_;

  public:
    static Box < FileReadStream > Construct(wchar_t const* path, bool shared) noexcept;
    FileReadStream(FileReadStream const&) = delete;
    FileReadStream(FileReadStream && other) noexcept;
    FileReadStream & operator = (FileReadStream const &) = delete;
    FileReadStream & operator = (FileReadStream && other) noexcept;
    std::int64_t Read(void * buffer, int count) noexcept;
    bool Seek(std::int64_t offset, SeekPosition position) noexcept;

    ~FileReadStream() {
      Destroy();
    }

    std::int64_t Tell() const noexcept {
      return handle_offset_ - valid_count_;
    }

  private:
    void Destroy() noexcept;

    FileReadStream(void * handle, char * buffer) noexcept {
      handle_ = handle;
      handle_offset_ = 0;
      buffer_ = buffer;
      buffer_offset_ = 0;
      valid_count_ = 0;
    }
  };
}

#endif