#pragma once

#include <cstdint>
#include <optional>

#include "MMWave/Configured/PeopleTracking/Tlv/TlvTypes.hpp"
#include "MMWave/Streaming/Frame.hpp"

/// Sarwave.Integration.PeopleTracking: the People Tracking output put together with the IMU --
/// which targets and points a session holds, moved with the device as it turns -- plus the IMU
/// glue the apps share (ImuFrames) and streaming over the LAN (Lan/).
namespace Sarwave::Integration::PeopleTracking {

namespace Streaming = Core::MMWave::Streaming;
namespace Tlv = Core::MMWave::Configured::PeopleTracking::Tlv;

/// The People Tracking TLVs of one frame, each present or not. Views into the frame: only valid
/// while it is.
struct EntryPackage {
    std::optional<Tlv::TlvPointCloud> pointCloud;
    std::optional<Tlv::TlvContentRange<Tlv::TlvTargetList>> targets;
    std::optional<Tlv::TlvContentRange<Tlv::TlvTargetHeight>> heights;
    std::optional<Tlv::TlvContentRange<Tlv::TlvTargetIndex>> indexes;
    std::optional<bool> presence;
    std::uint32_t frameNumber = 0;

    /// Sorts the frame's TLVs into a package, checking on the way that the frame is one the
    /// device can have sent. Every rule here held for all 2,360 intact frames in the recordings
    /// described in docs/recorded_data.txt.
    ///
    /// Throws Core::MMWave::Exceptions::FrameEntryMalformedException when the frame breaks one
    /// of those rules.
    static EntryPackage unpack(const Streaming::Frame& frame);
};

}  // namespace Sarwave::Integration::PeopleTracking
