#pragma once

#include <cassert>

#include "MMWave/Tlv/TlvCore.hpp"
#include "../../Tlv/TlvCore.hpp"

namespace MMWave::Configured::OutOfBox {
    // TLV header + known payload types for the 3D People Tracking demo output
    // format, per the official "3D People Tracking User's Guide" UART Output
    // Data Format section.
#pragma pack(push, 1)
    struct DetectedPoint {
        // Out of Box demo's plain detected-point format: Cartesian XYZ + radial
        // velocity. NOT used by this demo's Point Cloud TLV -- kept only for
        // compatibility if you ever go back to flashing out_of_box_6843_aop.bin.
        float x;
        float y;
        float z;
        float velocity;

        // Iterable range of the legacy Out of Box DetectedPoint format. Only
        // meaningful when type == TLV_DETECTED_POINTS (i.e. not this demo).
        struct DetectedPointRange {
            const uint8_t* payload;
            uint32_t length;

            [[nodiscard]] uint32_t getCount() const {
                return length / sizeof(DetectedPoint);
            }

            [[nodiscard]] const DetectedPoint* begin() const {
                return reinterpret_cast<const DetectedPoint*>(payload);
            }
            [[nodiscard]] const DetectedPoint* end() const {
                return begin() + (length / sizeof(DetectedPoint));
            }

            [[nodiscard]] const DetectedPoint& operator[](const uint32_t i) const {
                assert(i < getCount());
                return begin()[i];
            }
        };

        [[nodiscard]] static DetectedPointRange range(const Tlv::TlvEntry& entry) {
            return {entry.payload, entry.length};
        }
    };
#pragma pack(pop)
}