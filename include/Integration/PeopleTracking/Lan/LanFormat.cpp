#include "Integration/PeopleTracking/Lan/LanFormat.hpp"

#include <algorithm>
#include <bit>
#include <string>

#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Common/Numerics.hpp"
#include "MMWave/Recording/RecordingFormat.hpp"

namespace Sarwave::Integration::PeopleTracking::Lan {

using Core::InvalidDataException;
using Core::Imu::ImuSample;
using Core::MMWave::Vector3;

namespace {

constexpr std::string_view Magic = "SWLN";
constexpr std::uint16_t Version = 1;
constexpr int ImuSampleSize = 8 + 7 * 4;

void writeLe(std::uint8_t* at, std::uint64_t value, int bytes) {
    for (int i = 0; i < bytes; ++i) at[i] = static_cast<std::uint8_t>(value >> (8 * i));
}

std::uint64_t readLe(const std::uint8_t* at, int bytes) {
    std::uint64_t value = 0;
    for (int i = 0; i < bytes; ++i) value |= static_cast<std::uint64_t>(at[i]) << (8 * i);
    return value;
}

void writeSingle(std::uint8_t* at, float value) {
    writeLe(at, std::bit_cast<std::uint32_t>(value), 4);
}

float readSingle(const std::uint8_t* at) {
    return std::bit_cast<float>(static_cast<std::uint32_t>(readLe(at, 4)));
}

void writeVector(std::uint8_t* at, Vector3 value) {
    writeSingle(at, value.x);
    writeSingle(at + 4, value.y);
    writeSingle(at + 8, value.z);
}

Vector3 readVector(const std::uint8_t* at) {
    return {readSingle(at), readSingle(at + 4), readSingle(at + 8)};
}

/// A message of `type` with room for `payloadLength` bytes after the header.
std::vector<std::uint8_t> message(std::uint8_t type, std::size_t payloadLength) {
    std::vector<std::uint8_t> bytes(LanFormat::HeaderSize + payloadLength);
    bytes[0] = type;
    writeLe(bytes.data() + 1, static_cast<std::uint32_t>(payloadLength), 4);
    return bytes;
}

}  // namespace

std::vector<std::uint8_t> LanFormat::encodeHello(std::string_view cfgText) {
    auto bytes = message(Hello, Magic.size() + 2 + cfgText.size());
    std::uint8_t* payload = bytes.data() + HeaderSize;
    std::ranges::copy(Magic, payload);
    writeLe(payload + Magic.size(), Version, 2);
    std::ranges::copy(cfgText, payload + Magic.size() + 2);
    return bytes;
}

std::vector<std::uint8_t> LanFormat::encodeFrame(std::span<const std::uint8_t> frame,
                                                 TimeSpan timestamp) {
    auto bytes = message(Frame, TimestampSize + frame.size());
    writeLe(bytes.data() + HeaderSize, static_cast<std::uint64_t>(timestamp.count()), 8);
    std::ranges::copy(frame, bytes.begin() + HeaderSize + TimestampSize);
    return bytes;
}

std::vector<std::uint8_t> LanFormat::encodeMalformed(
    const Core::MMWave::Exceptions::FrameMalformedException& exception, TimeSpan timestamp) {
    const auto record = Core::MMWave::Recording::RecordingFormat::encodeMalformed(exception);
    auto bytes = message(Malformed, TimestampSize + record.size());
    writeLe(bytes.data() + HeaderSize, static_cast<std::uint64_t>(timestamp.count()), 8);
    std::ranges::copy(record, bytes.begin() + HeaderSize + TimestampSize);
    return bytes;
}

std::vector<std::uint8_t> LanFormat::encodeImu(std::span<const ImuSample> samples) {
    auto bytes = message(Imu, 4 + samples.size() * ImuSampleSize);
    std::uint8_t* at = bytes.data() + HeaderSize;
    writeLe(at, static_cast<std::uint32_t>(samples.size()), 4);
    at += 4;
    for (const ImuSample& sample : samples) {
        writeLe(at, static_cast<std::uint64_t>(sample.timestamp.count()), 8);
        writeVector(at + 8, sample.acceleration);
        writeVector(at + 20, sample.angularRate);
        writeSingle(at + 32, sample.temperature);
        at += ImuSampleSize;
    }
    return bytes;
}

void LanFormat::readHeader(std::span<const std::uint8_t, HeaderSize> header, std::uint8_t& type,
                           std::int32_t& length) {
    type = header[0];
    length = static_cast<std::int32_t>(static_cast<std::uint32_t>(readLe(header.data() + 1, 4)));
    if (type < Hello || type > Imu || length < 0 || length > MaxPayload) {
        throw InvalidDataException("Not a Sarwave LAN message (type " + std::to_string(type) +
                                   ", length " + std::to_string(length) + ").");
    }
}

std::string LanFormat::decodeHello(std::span<const std::uint8_t> payload) {
    const std::size_t fixedSize = Magic.size() + 2;
    if (payload.size() < fixedSize || !std::ranges::equal(payload.first(Magic.size()), Magic))
        throw InvalidDataException("The other end is not a Sarwave sender.");

    const auto version = static_cast<std::uint16_t>(readLe(payload.data() + Magic.size(), 2));
    if (version != Version) {
        throw InvalidDataException("The sender speaks version " + std::to_string(version) +
                                   "; this receiver speaks " + std::to_string(Version) + ".");
    }

    return {payload.begin() + static_cast<std::ptrdiff_t>(fixedSize), payload.end()};
}

TimeSpan LanFormat::readTimestamp(std::span<const std::uint8_t> payload) {
    if (payload.size() < static_cast<std::size_t>(TimestampSize))
        throw InvalidDataException("A message is too short for its timestamp.");
    return TimeSpan(static_cast<std::int64_t>(readLe(payload.data(), 8)));
}

int LanFormat::decodeImu(std::span<const std::uint8_t> payload, std::vector<ImuSample>& samples) {
    const std::int64_t count =
        payload.size() >= 4
            ? static_cast<std::int32_t>(static_cast<std::uint32_t>(readLe(payload.data(), 4)))
            : -1;
    if (count < 0 || static_cast<std::int64_t>(payload.size()) != 4 + count * ImuSampleSize)
        throw InvalidDataException("An IMU message's length does not match its sample count.");

    if (samples.size() < static_cast<std::size_t>(count))
        samples.resize(static_cast<std::size_t>(count));

    const std::uint8_t* at = payload.data() + 4;
    for (std::int64_t i = 0; i < count; ++i) {
        samples[static_cast<std::size_t>(i)] = {TimeSpan(static_cast<std::int64_t>(readLe(at, 8))),
                                                readVector(at + 8), readVector(at + 20),
                                                readSingle(at + 32)};
        at += ImuSampleSize;
    }
    return static_cast<int>(count);
}

}  // namespace Sarwave::Integration::PeopleTracking::Lan
