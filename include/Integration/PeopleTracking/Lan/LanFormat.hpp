#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "Imu/ImuSample.hpp"
#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Exceptions/FrameMalformedException.hpp"

namespace Sarwave::Integration::PeopleTracking::Lan {

using Core::MMWave::TimeSpan;

/// What goes over the connection, all little endian. Each message is a type byte, an int32
/// payload length and the payload:
///
///   Hello      "SWLN", uint16 version, then the cfg text (UTF-8) the sender's radar runs
///   Frame      int64 timestamp ticks, then the frame exactly as the radar sent it
///   Malformed  int64 timestamp ticks, then the frame recording's malformed-frame record
///   Imu        int32 count, then per sample: int64 timestamp ticks, accel x/y/z (g),
///              gyro x/y/z (°/s), temperature (°C) as floats; raw, in the IMU's own axes
///
/// Timestamps are on the sender's capture clock, the one its frames and IMU samples share. The
/// sender says Hello before anything else on every connection. Raw data is sent, as in the
/// recordings, so the receiver processes it the way a replay does.
///
/// The encoders return the whole message, header included.
struct LanFormat {
    static constexpr std::uint8_t Hello = 1;
    static constexpr std::uint8_t Frame = 2;
    static constexpr std::uint8_t Malformed = 3;
    static constexpr std::uint8_t Imu = 4;

    static constexpr int HeaderSize = 1 + 4;

    /// Where a Frame or Malformed payload's content starts, after its timestamp.
    static constexpr int TimestampSize = 8;

    /// Larger than any message the sender makes; anything bigger means the stream is out of step.
    static constexpr int MaxPayload = 1 << 20;

    static std::vector<std::uint8_t> encodeHello(std::string_view cfgText);
    static std::vector<std::uint8_t> encodeFrame(std::span<const std::uint8_t> frame,
                                                 TimeSpan timestamp);
    static std::vector<std::uint8_t> encodeMalformed(
        const Core::MMWave::Exceptions::FrameMalformedException& exception, TimeSpan timestamp);
    static std::vector<std::uint8_t> encodeImu(std::span<const Core::Imu::ImuSample> samples);

    /// Throws InvalidDataException when the header cannot be one of this format's.
    static void readHeader(std::span<const std::uint8_t, HeaderSize> header, std::uint8_t& type,
                           std::int32_t& length);

    /// The sender's cfg text. Throws InvalidDataException when the sender speaks another format
    /// or version.
    static std::string decodeHello(std::span<const std::uint8_t> payload);

    /// For a Frame or Malformed payload: its timestamp. Throws InvalidDataException when the
    /// payload is too short to hold one.
    static TimeSpan readTimestamp(std::span<const std::uint8_t> payload);

    /// Decodes an Imu payload into `samples`, growing it if needed; returns how many samples it
    /// holds. Throws InvalidDataException when the payload's length does not match its count.
    static int decodeImu(std::span<const std::uint8_t> payload,
                         std::vector<Core::Imu::ImuSample>& samples);
};

}  // namespace Sarwave::Integration::PeopleTracking::Lan
