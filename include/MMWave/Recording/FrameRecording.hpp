#pragma once

#include <filesystem>
#include <istream>
#include <memory>
#include <span>
#include <vector>

#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Exceptions/FrameMalformedException.hpp"
#include "MMWave/Streaming/Frame.hpp"

namespace Sarwave::Core::MMWave::Recording {

/// A whole recording loaded into memory: every frame and its timestamp, in recorded order. Use
/// it directly to run frames through a pipeline as fast as possible, or hand it to FramePlayback
/// to get them back at their original pace.
///
/// The recording owns the frames; don't use one after the recording is disposed.
class FrameRecording {
   public:
    /// A malformed frame noted with FrameRecorder::recordMalformed.
    struct MalformedFrame {
        /// Where it falls among frames(): the index of the frame recorded next, or count() if
        /// none was.
        int frameIndex = 0;

        /// The timestamp it was noted with.
        TimeSpan timestamp{};

        /// Rebuilt from the recording: same type, cause, frame number and detail.
        std::shared_ptr<const Exceptions::FrameMalformedException> exception;
    };

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

    /// The malformed frames that were noted, in the order they were recorded. A frame that the
    /// reader skipped appears only here. A frame that arrived and was then rejected is usually
    /// also in frames(), just before the position its entry here gives.
    [[nodiscard]] std::span<const MalformedFrame> malformedFrames() const {
        return m_malformedFrames;
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
    /// that is not a recording, an unknown version, or a corrupt record. Reads versions 1 and 2.
    static FrameRecording load(const std::filesystem::path& path);

    /// Reads from the stream's current position to its end.
    static FrameRecording load(std::istream& stream);

    /// Frees the frames.
    void dispose();

   private:
    FrameRecording(std::vector<Streaming::Frame> frames, std::vector<TimeSpan> timestamps,
                   std::vector<MalformedFrame> malformedFrames, bool isTruncated)
        : m_frames(std::move(frames)),
          m_timestamps(std::move(timestamps)),
          m_malformedFrames(std::move(malformedFrames)),
          m_isTruncated(isTruncated) {
    }

    std::vector<Streaming::Frame> m_frames;
    std::vector<TimeSpan> m_timestamps;
    std::vector<MalformedFrame> m_malformedFrames;
    bool m_isTruncated = false;
};

}  // namespace Sarwave::Core::MMWave::Recording
