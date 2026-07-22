#ifndef MTTLIB_INCLUDE_FILE_INPUT_STREAM_H_
#define MTTLIB_INCLUDE_FILE_INPUT_STREAM_H_

#include <cstdint>

#include "box.h"

namespace mttlib {
  enum class SeekPosition {
    kBeginning,
    kCurrent,
    kEnd
  };

  class FileInputStream {
    friend class Box < FileInputStream >;

    static constexpr int kBufferSize = 4096;

    void *handle_;
    std::int64_t handle_offset_;
    char *buffer_;
    int buffer_offset_;
    int valid_count_;

    void Destroy() noexcept;

    FileInputStream(void *handle, char *buffer) noexcept {
      handle_ = handle;
      handle_offset_ = 0;
      buffer_ = buffer;
      buffer_offset_ = 0;
      valid_count_ = 0;
    }

  public:
    // assert(path != nullptr)
    static Box < FileInputStream > Construct(const char *path) noexcept;
    // assert(path != nullptr)
    static Box < FileInputStream > Construct(const wchar_t *path) noexcept;
    FileInputStream(const FileInputStream &) = delete;
    FileInputStream(FileInputStream &&other) noexcept;
    FileInputStream &operator = (const FileInputStream &) = delete;
    FileInputStream &operator = (FileInputStream &&) noexcept;
    // assert(buffer != nullptr && count >= 0)
    int Read(char *buffer, int count) noexcept;
    bool Seek(std::int64_t offset, SeekPosition position) noexcept;

    ~FileInputStream() {
      Destroy();
    }

    std::int64_t Tell() const noexcept {
      return handle_offset_ - valid_count_;
    }
  };
}

#endif