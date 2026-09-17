#pragma once
#include <boost/asio.hpp>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
#include "Porter.hpp"

namespace MMWave::Streaming {

    // Fixed "here's where a new frame starts" marker.
    inline constexpr uint8_t MAGIC_WORD[8] = {0x02, 0x01, 0x04, 0x03, 0x06, 0x05, 0x08, 0x07};

    // Fixed header immediately following the magic word in every frame.
    // Packed with no padding -- memcpy'd directly from the raw byte stream.
#pragma pack(push, 1)
    struct FrameHeader {
        uint32_t version;
        uint32_t totalPacketLen;
        uint32_t platform;
        uint32_t frameNumber;
        uint32_t timeCpuCycles;
        uint32_t numDetectedObj;
        uint32_t numTLVs;
        uint32_t subFrameNumber;
    };
#pragma pack(pop)

    // A single complete frame: the parsed header, plus the frame's raw bytes
    // (magic word + header + all TLVs), exactly `header.totalPacketLen` long.
    // Tlv.hpp's TlvRange consumes `bytes` directly.
    struct Frame {
        FrameHeader header;
        std::vector<uint8_t> bytes;
    };

    // Reads frames from a Data port one at a time. Owns all buffering state
    // internally -- callers just keep calling readNextFrame() in a loop and
    // never need to touch the byte-level details.
    class FrameReader {
    public:
        explicit FrameReader(Porter::Context& ctx) : ctx_(ctx) {}

        // Blocks until one complete frame has been read, then returns it.
        // Throws std::runtime_error if the Data port read fails.
        Frame readNextFrame() {
            while (true) {
                readByte();

                if (checkFrameSync()) {
                    headerParsed_ = false;
                }

                if (!headerParsed_) {
                    tryParseHeader();
                }

                if (headerParsed_ && buffer_.size() >= currentHeader_.totalPacketLen) {
                    return extractFrame();
                }
            }
        }

    private:
        void readByte() {
            uint8_t byte;
            boost::system::error_code ec;
            std::size_t n = ctx_.data.read_some(boost::asio::buffer(&byte, 1), ec);

            if (ec) {
                throw std::runtime_error("Data port read error: " + ec.message());
            }
            if (n == 0) {
                return;
            }
            buffer_.push_back(byte);
        }

        // Checks whether the buffer's last 8 bytes match the magic word; if
        // so, trims everything before it so the buffer starts cleanly at the
        // beginning of a frame. Returns true when a new boundary was found.
        bool checkFrameSync() {
            if (buffer_.size() < sizeof(MAGIC_WORD)) {
                return false;
            }

            const uint8_t* tail = buffer_.data() + buffer_.size() - sizeof(MAGIC_WORD);
            if (std::memcmp(tail, MAGIC_WORD, sizeof(MAGIC_WORD)) != 0) {
                return false;
            }

            std::size_t frameStart = buffer_.size() - sizeof(MAGIC_WORD);
            if (frameStart > 0) {
                const auto offset = static_cast<std::vector<uint8_t>::difference_type>(frameStart);
                buffer_.erase(buffer_.begin(), buffer_.begin() + offset);
            }
            return true;
        }

        // Once enough bytes have arrived to cover the fixed header, copies
        // it into currentHeader_ and marks headerParsed_.
        void tryParseHeader() {
            if (buffer_.size() < sizeof(MAGIC_WORD) + sizeof(FrameHeader)) {
                return;
            }
            std::memcpy(&currentHeader_, buffer_.data() + sizeof(MAGIC_WORD), sizeof(FrameHeader));
            headerParsed_ = true;
        }

        // Slices out exactly the completed frame's bytes, leaving any
        // leftover (already-arrived bytes of the next frame) in buffer_.
        Frame extractFrame() {
            Frame frame;
            frame.header = currentHeader_;

            const auto len = static_cast<std::vector<uint8_t>::difference_type>(currentHeader_.totalPacketLen);
            frame.bytes.assign(buffer_.begin(), buffer_.begin() + len);
            buffer_.erase(buffer_.begin(), buffer_.begin() + len);

            headerParsed_ = false;
            return frame;
        }

        Porter::Context& ctx_;
        std::vector<uint8_t> buffer_;
        bool headerParsed_ = false;
        FrameHeader currentHeader_{};
    };
}