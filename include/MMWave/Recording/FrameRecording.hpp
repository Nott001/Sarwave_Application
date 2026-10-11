#pragma once

#include <filesystem>
#include <istream>
#include <span>
#include <vector>

#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Streaming/Frame.hpp"

namespace Sarwave::Core::MMWave::Recording {

/// A whole recording loaded into memory: every frame and its timestamp, in recorded order. Use
/// it directly to run frames through a pipeline as fast as possible, or hand it to FramePlayback
/// to get them back at their original pace.
///
/// The recording owns the frames; don't use one after the recording is disposed.
class FrameRecording {
   public:
    FrameRecording(FrameRecording&&) noexcept = default;
    FrameRecording& operator=(FrameRecording&&) noexcept = default;

    [[nodiscard]] int count() const {
        return static_cast<int>(m_frames.size());
    }

    /// The frames, oldest first.
    [[nodiscard]] std::span<const Streaming::Frame> frames() const {
        return m_frames;
    }

    /// The timestamp given to FrameRecorder::record for the frame at the same index.
    [[nodiscard]] std::span<const TimeSpan> timestamps() const {
        return m_timestamps;
    }

    /// Time from the first frame to the last.
    [[nodiscard]] TimeSpan duration() const {
        return m_timestamps.empty() ? TimeSpan::zero() : m_timestamps.back() - m_timestamps.front();
    }

    /// True if the file ended part-way through a frame (the recorder was killed mid-write).
    /// Everything before that point is still loaded.
    [[nodiscard]] bool isTruncated() const {
        return m_isTruncated;
    }

    /// Throws FileNotFoundException for a missing file, InvalidDataException for something
    /// that is not a recording, an unknown version, or a corrupt frame.
    static FrameRecording load(const std::filesystem::path& path);

    /// Reads from the stream's current position to its end.
    static FrameRecording load(std::istream& stream);

    /// Frees the frames.
    void dispose();

   private:
    FrameRecording(std::vector<Streaming::Frame> frames, std::vector<TimeSpan> timestamps,
                   bool isTruncated)
        : m_frames(std::move(frames)),
          m_timestamps(std::move(timestamps)),
          m_isTruncated(isTruncated) {
    }

    std::vector<Streaming::Frame> m_frames;
    std::vector<TimeSpan> m_timestamps;
    bool m_isTruncated = false;
};

}  // namespace Sarwave::Core::MMWave::Recording
