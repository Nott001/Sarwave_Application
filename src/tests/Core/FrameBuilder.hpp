#pragma once

// Builds raw People Tracking frames for tests, laid out as the device sends them: padded to a
// multiple of 32 bytes, with numDetectedObj matching the point cloud.

#include <cstdint>
#include <cstring>
#include <vector>

#include "MMWave/Streaming/Frame.hpp"
#include "MMWave/Streaming/StreamingConstants.hpp"

namespace SarwaveTest {

class FrameBuilder {
   public:
    FrameBuilder& tlv(std::uint32_t type, const std::vector<std::uint8_t>& payload) {
        put32(m_body, type);
        put32(m_body, static_cast<std::uint32_t>(payload.size()));
        m_body.insert(m_body.end(), payload.begin(), payload.end());
        ++m_count;
        return *this;
    }

    FrameBuilder& presence(bool present) {
        std::vector<std::uint8_t> p;
        put32(p, present ? 1 : 0);
        return tlv(1021, p);
    }

    FrameBuilder& targets(const std::vector<std::uint8_t>& ids, float x = 1.0f, float y = 2.0f) {
        std::vector<std::uint8_t> p;
        for (std::uint8_t id : ids) {
            p.push_back(id);
            p.insert(p.end(), 3, 0);
            const float values[27] = {x, y, 0.5f, 0.1f, 0.2f, 0, 0, 0, 0};
            for (float v : values) putF(p, v);
        }
        return tlv(1010, p);
    }

    FrameBuilder& heights(const std::vector<std::uint8_t>& ids, float maxZ, float minZ) {
        std::vector<std::uint8_t> p;
        for (std::uint8_t id : ids) {
            p.push_back(id);
            p.insert(p.end(), 3, 0);
            putF(p, maxZ);
            putF(p, minZ);
        }
        return tlv(1012, p);
    }

    FrameBuilder& indexes(const std::vector<std::uint8_t>& owners) {
        return tlv(1011, owners);
    }

    /// `n` identical points 2 m ahead.
    FrameBuilder& cloud(int n) {
        m_points = static_cast<std::uint32_t>(n);
        std::vector<std::uint8_t> p;
        for (float unit : {0.01f, 0.01f, 0.05f, 0.00025f, 0.04f}) putF(p, unit);
        for (int i = 0; i < n; ++i) {
            p.push_back(0);                             // elevation
            p.push_back(static_cast<std::uint8_t>(i));  // azimuth
            put16(p, 10);                               // doppler
            put16(p, 8000);                             // range: 2 m
            put16(p, 300);                              // snr
        }
        return tlv(1020, p);
    }

    /// Overrides the header's numDetectedObj (set by cloud() otherwise).
    FrameBuilder& detectedObjects(std::uint32_t n) {
        m_points = n;
        return *this;
    }

    /// Leaves out the padding, as frames written before the reader checked alignment were.
    FrameBuilder& unpadded() {
        m_pad = false;
        return *this;
    }

    std::vector<std::uint8_t> bytes(std::uint32_t frameNumber = 0) const {
        std::vector<std::uint8_t> out(
            Sarwave::Core::MMWave::Streaming::StreamingConstants::MagicWord.begin(),
            Sarwave::Core::MMWave::Streaming::StreamingConstants::MagicWord.end());
        std::uint32_t total = static_cast<std::uint32_t>(40 + m_body.size());
        if (m_pad) total = (total + 31) / 32 * 32;
        for (std::uint32_t v : {1u, total, 2u, frameNumber, 0u, m_points, m_count, 0u})
            put32(out, v);
        out.insert(out.end(), m_body.begin(), m_body.end());
        out.resize(total, 0x0F);  // the demo pads with 0x0F
        return out;
    }

    Sarwave::Core::MMWave::Streaming::Frame frame(std::uint32_t frameNumber = 0) const {
        const auto b = bytes(frameNumber);
        return Sarwave::Core::MMWave::Streaming::Frame::copyFrom(b);
    }

   private:
    static void put32(std::vector<std::uint8_t>& v, std::uint32_t x) {
        for (int i = 0; i < 4; ++i) v.push_back(static_cast<std::uint8_t>(x >> (8 * i)));
    }
    static void put16(std::vector<std::uint8_t>& v, std::uint16_t x) {
        v.push_back(static_cast<std::uint8_t>(x));
        v.push_back(static_cast<std::uint8_t>(x >> 8));
    }
    static void putF(std::vector<std::uint8_t>& v, float f) {
        std::uint32_t x;
        std::memcpy(&x, &f, 4);
        put32(v, x);
    }

    std::vector<std::uint8_t> m_body;
    std::uint32_t m_count = 0;
    std::uint32_t m_points = 0;
    bool m_pad = true;
};

}  // namespace SarwaveTest
