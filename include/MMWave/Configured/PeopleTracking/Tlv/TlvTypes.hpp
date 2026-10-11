#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <span>
#include <string>

#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Common/Numerics.hpp"
#include "MMWave/Configured/PeopleTracking/Pointing/Pointing.hpp"
#include "MMWave/Tlv/TlvContentRange.hpp"
#include "MMWave/Tlv/TlvEntry.hpp"

/// The TLVs the 3D People Tracking demo sends, and their record layouts.
namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Tlv {

using MMWave::Tlv::TlvContentRange;
using MMWave::Tlv::TlvEntry;

enum class TlvType : std::uint32_t {
    TlvTargetList = 1010,
    TlvTargetIndex = 1011,
    TlvTargetHeight = 1012,
    TlvPointCloud = 1020,
    TlvPresenceIndication = 1021,
};

/// Enum.ToString(): the member name, or the number for a value with no name.
std::string toString(TlvType type);

/// A track id on the wire: one byte. 253..255 mean "not associated" (see PointNotAssociated).
struct TlvTargetIndex {
    std::uint8_t index = 0;

    [[nodiscard]] bool isAssociated() const {
        using Pointing::PointNotAssociated;
        switch (static_cast<PointNotAssociated>(index)) {
            case PointNotAssociated::PointLikelyNoise:
            case PointNotAssociated::PointOutOfBounds:
            case PointNotAssociated::PointSnrWeak:
                return false;
            default:
                return true;
        }
    }

    [[nodiscard]] std::string toString() const {
        return std::to_string(index);
    }

    explicit operator Pointing::PointNotAssociated() const {
        return static_cast<Pointing::PointNotAssociated>(index);
    }

    friend constexpr bool operator==(TlvTargetIndex, TlvTargetIndex) = default;
};
static_assert(sizeof(TlvTargetIndex) == 1);

/// Target Height TLV (1012) record. The user guide documents targetID as a single byte (a
/// 9-byte record), but captured frames show a 12-byte stride: the same 4-byte slot the target
/// list uses for its tid. Only the low byte of that slot is read.
struct TlvTargetHeight {
    TlvTargetIndex targetId;
    std::uint8_t padding[3];
    float maxZ;
    float minZ;
};
static_assert(sizeof(TlvTargetHeight) == 12);
static_assert(offsetof(TlvTargetHeight, maxZ) == 4);

/// Target List TLV (1010) record (trackerProc_Target): 112 bytes.
struct TlvTargetList {
    TlvTargetIndex targetId;
    std::uint8_t padding[3];
    float posX;
    float posY;
    float posZ;
    float velX;
    float velY;
    float velZ;
    float accX;
    float accY;
    float accZ;
    float ec[16];
    float g;
    float confidenceLevel;

    [[nodiscard]] Vector3 pos() const {
        return {posX, posY, posZ};
    }

    [[nodiscard]] Vector3 vel() const {
        return {velX, velY, velZ};
    }

    [[nodiscard]] Vector3 acc() const {
        return {accX, accY, accZ};
    }

    /// Ec as a 4x4 matrix, ASSUMED row-major (ec[0..3] is the first row), the same assumption
    /// the C# code makes; the documentation does not say.
    [[nodiscard]] Matrix4x4 ecMatrix() const {
        Matrix4x4 matrix;
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) matrix.m[r][c] = ec[r * 4 + c];
        }
        return matrix;
    }
};
static_assert(sizeof(TlvTargetList) == 112);
static_assert(offsetof(TlvTargetList, ec) == 40);
static_assert(offsetof(TlvTargetList, g) == 104);

/// Point Cloud TLV (1020): a PointUnit, then CompressedPoint records.
struct TlvPointCloud {
    Pointing::PointUnit unit{};
    TlvContentRange<Pointing::CompressedPoint> points;

    /// Throws ArgumentOutOfRangeException when the payload is too short to hold the PointUnit,
    /// as MemoryMarshal.AsRef does in the C# version.
    static TlvPointCloud decompile(const TlvEntry& entry) {
        return decompile(entry.bytes);
    }

    static TlvPointCloud decompile(std::span<const std::uint8_t> bytes) {
        if (bytes.size() < sizeof(Pointing::PointUnit)) throw ArgumentOutOfRangeException("length");

        TlvPointCloud cloud;
        std::memcpy(&cloud.unit, bytes.data(), sizeof(Pointing::PointUnit));
        cloud.points =
            TlvContentRange<Pointing::CompressedPoint>(bytes.subspan(sizeof(Pointing::PointUnit)));
        return cloud;
    }
};

/// Presence Indication TLV (1021): one little-endian uint32, non-zero = someone is present.
struct TlvPresenceIndication {
    /// Throws ArgumentOutOfRangeException for a payload shorter than 4 bytes, as
    /// BinaryPrimitives.ReadUInt32LittleEndian does.
    static bool presence(const TlvEntry& entry) {
        if (entry.bytes.size() < 4) throw ArgumentOutOfRangeException("length");
        const auto& b = entry.bytes;
        const std::uint32_t value =
            static_cast<std::uint32_t>(b[0]) | static_cast<std::uint32_t>(b[1]) << 8 |
            static_cast<std::uint32_t>(b[2]) << 16 | static_cast<std::uint32_t>(b[3]) << 24;
        return value != 0;
    }
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Tlv

template <>
struct std::hash<Sarwave::Core::MMWave::Configured::PeopleTracking::Tlv::TlvTargetIndex> {
    std::size_t operator()(
        Sarwave::Core::MMWave::Configured::PeopleTracking::Tlv::TlvTargetIndex id) const noexcept {
        return id.index;
    }
};
