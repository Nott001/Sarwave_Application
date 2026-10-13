#pragma once

#include <filesystem>
#include <memory>
#include <mutex>
#include <ostream>

#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Streaming/Frame.hpp"

namespace Sarwave::Core::MMWave::Recording {

/// Writes frames to a file (or any stream) so they can be loaded again with FrameRecording. It
/// only needs the Frame and a timestamp, so it hooks in wherever frames are already handled --
/// typically one extra line inside a ConstantCapture callback.
///
/// The frame is copied out during record(); nothing is retained afterwards. Each frame reaches
/// the file as soon as it is recorded, so the recording is complete up to the last frame even if
/// the process is killed before dispose() runs. Safe to call from the capture thread while
/// another thread disposes it.
class FrameRecorder {
   public:
    /// Creates the file, replacing any existing one. Throws IOException if it can't.
    explicit FrameRecorder(const std::filesystem::path& path);

    /// Writes to `stream`. Owned (and closed with this recorder) unless `leaveOpen`.
    explicit FrameRecorder(std::unique_ptr<std::ostream> stream);
    FrameRecorder(std::ostream& stream, bool leaveOpen);

    ~FrameRecorder();
    FrameRecorder(const FrameRecorder&) = delete;
    FrameRecorder& operator=(const FrameRecorder&) = delete;

    /// Frames written so far.
    [[nodiscard]] int frameCount() const;

    /// timestamp: when the frame arrived, on whatever clock the caller uses. Playback
    /// reproduces the gaps between these, so they should not go backwards.
    /// Throws ObjectDisposedException after dispose().
    void record(const Streaming::Frame& frame, TimeSpan timestamp);

    /// Pushes buffered frames to the underlying stream.
    void flush();

    void dispose();

   private:
    void writeHeader();
    void write(std::span<const std::uint8_t> bytes);
    void throwIfDisposed() const;

    mutable std::mutex m_gate;
    std::unique_ptr<std::ostream> m_owned;
    std::ostream* m_stream;
    bool m_leaveOpen = false;
    bool m_disposed = false;
    int m_frameCount = 0;
};

}  // namespace Sarwave::Core::MMWave::Recording
