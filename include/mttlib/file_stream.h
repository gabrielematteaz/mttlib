#ifndef MTTLIB_INCLUDE_FILE_STREAM_H_
#define MTTLIB_INCLUDE_FILE_STREAM_H_

#include <cstdint>

#include "box.h"
#include "stream.h"

namespace mttlib {
  class FileStream : public Stream {
    static constexpr int kBufferSize = 4096;

    void * handle_;
    std::int64_t handle_offset_;
    char * buffer_;
    int buffer_offset_;
    int valid_count_;

  public:
    static Box < FileStream > Construct(wchar_t const* path, bool shared) noexcept;
    static Box < FileStream > Construct(char const* path, bool shared) noexcept;
    FileStream() noexcept;
    FileStream(FileStream const&) = delete;
    FileStream(FileStream && other) noexcept;
    FileStream & operator = (FileStream const&) = delete;
    FileStream & operator = (FileStream && other) noexcept;
    std::int64_t Read(void * buffer, std::int64_t count) noexcept override;
    std::int64_t Seek(std::int64_t offset, SeekPosition position) noexcept override;

    ~FileStream() {
      Destroy();
    }

    std::int64_t Tell() noexcept override {
      return handle_offset_ - valid_count_;
    }

  private:
    void Destroy() noexcept;

    FileStream(void * handle, char * buffer) noexcept {
      handle_ = handle;
      handle_offset_ = 0;
      buffer_ = buffer;
      buffer_offset_ = 0;
      valid_count_ = 0;
    }
  };

  class WriteFileStream : public WriteStream {
    void * handle_;
    std::int64_t handle_offset_;

  public:
    static Box < WriteFileStream > Construct(wchar_t const* path, bool overwrite, bool shared) noexcept;
    static Box < WriteFileStream > Construct(char const* path, bool overwrite, bool shared) noexcept;
    WriteFileStream() noexcept;
    WriteFileStream(WriteFileStream const&) = delete;
    WriteFileStream(WriteFileStream && other) noexcept;
    WriteFileStream & operator = (WriteFileStream const&) = delete;
    WriteFileStream & operator = (WriteFileStream && other) noexcept;
    std::int64_t Write(void * buffer, std::int64_t count) noexcept override;
    std::int64_t Seek(std::int64_t offset, SeekPosition position) noexcept override;

    ~WriteFileStream() {
      Destroy();
    }

    std::int64_t Tell() noexcept override {
      return handle_offset_;
    }

  private:
    void Destroy() noexcept;

    WriteFileStream(void * handle) noexcept {
      handle_ = handle;
      handle_offset_ = 0;
    }
  };
}

#endif