#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>

/// Sarwave.Core.MMWave.Exceptions: what a damaged packet from the data port turns into.
namespace Sarwave::Core::MMWave::Exceptions {

/// A packet from the data port does not hold together. The serial link has no checksum and no
/// retransmission, so this is what bytes lost on the way end up looking like. The two derived
/// types say where it was noticed.
class FrameMalformedException : public std::runtime_error {
   public:
    explicit FrameMalformedException(std::string detail,
                                     std::optional<std::uint32_t> frameNumber = std::nullopt)
        : std::runtime_error(compose(detail, frameNumber)),
          m_detail(std::move(detail)),
          m_frameNumber(frameNumber) {
    }

    /// FrameHeader.frameNumber of the packet, or nullopt if its header could not be read. Taken
    /// from the damaged packet itself, so it is a hint rather than a certainty.
    [[nodiscard]] std::optional<std::uint32_t> frameNumber() const {
        return m_frameNumber;
    }

    /// What is wrong, without the frame number that what() puts in front.
    [[nodiscard]] const std::string& detail() const {
        return m_detail;
    }

   private:
    static std::string compose(const std::string& detail, std::optional<std::uint32_t> number) {
        return number ? "Frame " + std::to_string(*number) + ": " + detail : detail;
    }

    std::string m_detail;
    std::optional<std::uint32_t> m_frameNumber;
};

/// Thrown by FrameReader for a packet whose framing is broken, before any Frame is made from it.
/// The reader has already moved past the packet when this is thrown, so reading can simply be
/// continued.
class FrameReaderMalformedException : public FrameMalformedException {
   public:
    enum class Reason : std::int32_t {
        /// The header's TotalPacketLen is not a length a packet can have.
        Header,
        /// The TLVs counted by the header do not chain to the end of the packet.
        TlvChain,
        /// The next packet began before this one had reached its announced length.
        Interrupted,
    };

    FrameReaderMalformedException(Reason cause, std::string message,
                                  std::optional<std::uint32_t> frameNumber = std::nullopt)
        : FrameMalformedException(std::move(message), frameNumber), m_cause(cause) {
    }

    [[nodiscard]] Reason cause() const {
        return m_cause;
    }

   private:
    Reason m_cause;
};

/// Thrown by EntryPackage::unpack for a frame that passed the reader but whose contents are not
/// what the people tracking output can contain. Nothing has been applied from the frame when
/// this is thrown.
class FrameEntryMalformedException : public FrameMalformedException {
   public:
    enum class Reason : std::int32_t {
        /// Fewer TLVs fit in the packet than its header counts.
        Truncated,
        /// A TLV type that is not part of the output.
        UnknownTlv,
        /// The same TLV type twice in one frame.
        DuplicateTlv,
        /// A TLV whose length is not a whole number of its records.
        PayloadLength,
        /// The point cloud does not hold the number of points the header gives.
        PointCount,
        /// A track ID outside 0-249 in the target list or the target heights.
        TrackId,
        /// The target heights do not list the same targets as the target list.
        HeightMismatch,
        /// A target index that is neither a track ID nor one of the three markers.
        PointIndex,
        /// A presence value other than 0 or 1.
        Presence,
    };

    FrameEntryMalformedException(Reason cause, std::string message,
                                 std::optional<std::uint32_t> frameNumber = std::nullopt)
        : FrameMalformedException(std::move(message), frameNumber), m_cause(cause) {
    }

    [[nodiscard]] Reason cause() const {
        return m_cause;
    }

   private:
    Reason m_cause;
};

/// Enum.ToString(): the member name, or the number for a value with no name.
inline std::string toString(FrameReaderMalformedException::Reason reason) {
    using R = FrameReaderMalformedException::Reason;
    switch (reason) {
        case R::Header:
            return "Header";
        case R::TlvChain:
            return "TlvChain";
        case R::Interrupted:
            return "Interrupted";
    }
    return std::to_string(static_cast<std::int32_t>(reason));
}

/// Enum.ToString(): the member name, or the number for a value with no name.
inline std::string toString(FrameEntryMalformedException::Reason reason) {
    using R = FrameEntryMalformedException::Reason;
    switch (reason) {
        case R::Truncated:
            return "Truncated";
        case R::UnknownTlv:
            return "UnknownTlv";
        case R::DuplicateTlv:
            return "DuplicateTlv";
        case R::PayloadLength:
            return "PayloadLength";
        case R::PointCount:
            return "PointCount";
        case R::TrackId:
            return "TrackId";
        case R::HeightMismatch:
            return "HeightMismatch";
        case R::PointIndex:
            return "PointIndex";
        case R::Presence:
            return "Presence";
    }
    return std::to_string(static_cast<std::int32_t>(reason));
}

}  // namespace Sarwave::Core::MMWave::Exceptions
