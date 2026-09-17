#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <thread>
#include <vector>

#include "Porter.hpp"
#include "Streaming.hpp"

namespace MMWave::Streaming {

    // Reads frames from the Data port continuously on a background thread and
    // invokes a completion callback once per completely read frame.
    //
    // The capture is driven by the Context's own io_context::run(), so it can
    // be stopped cooperatively through serial_port::cancel(); there is never
    // an un-interruptible blocking read that stopConstantCapture() would have
    // to wait for.
    class ConstantCapture {
    public:
        using FrameCallback = std::function<void(const Streaming::Frame&)>;

        explicit ConstantCapture(Porter::Context& ctx) : ctx_(ctx) {}

        ConstantCapture(const ConstantCapture&) = delete;
        ConstantCapture& operator=(const ConstantCapture&) = delete;

        ~ConstantCapture() {
            stopConstantCapture();
        }

        // Starts the capture thread. onNewFrame is invoked on that thread once
        // per completely read frame, so handler work in the callback never
        // blocks the caller's own thread. Can only be called while the capture
        // is stopped.
        void beginConstantCapture(FrameCallback onNewFrame) {
            if (captureActive_.load()) {
                throw std::logic_error(
                    "ConstantCapture is already capturing frames");
            }

            onNewFrame_ = std::move(onNewFrame);
            buffer_.clear();
            headerParsed_ = false;
            currentHeader_ = {};
            staged_.fill(0);

            // A previous stopConstantCapture() may have stopped the io_context;
            // allow run() to be re-entered for this capture cycle.
            ctx_.io->restart();

            captureActive_.store(true);
            captureThread_ = std::thread([this]() { captureThreadFn(); });
        }

        // Stops the capture and waits for the capture thread to finish. Safe
        // to call from inside the frame callback itself; in that case the
        // current callback finishes and the thread winds down on its own.
        void stopConstantCapture() {
            if (!captureThread_.joinable()) {
                return;
            }

            const bool fromCaptureThread =
                captureThread_.get_id() == std::this_thread::get_id();

            captureActive_.store(false);

            if (fromCaptureThread) {
                // A callback runs between reads, so no read is pending to
                // cancel; the stop flag alone prevents the current handler
                // from issuing another read, and the thread unwinds on its
                // own.
                return;
            }

            // Abort any read that is still outstanding so its completion
            // handler fires promptly with operation_aborted instead of
            // lingering on a stale registration. cancel() does not report
            // whether a read was actually pending -- it succeeds in both
            // cases -- so the stop decision cannot rely on that.
            boost::system::error_code ec;
            ctx_.data.cancel(ec);
            if (!ec) {
                // The port is healthy: a handler may have just re-issued a
                // read before observing the stop flag. Stopping the
                // io_context guarantees run() returns, so the join below
                // cannot hang.
                ctx_.io->stop();
            }
            // A cancel failure means the Data port is already unusable and
            // any in-flight read completes with that same error (never
            // re-issuing), so run() unwinds on its own even without stop().
            captureThread_.join();
        }

        [[nodiscard]] bool isCapturing() const {
            return captureActive_.load();
        }

    private:
        void captureThreadFn() {
            try {
                issueRead();
                ctx_.io->run();
            } catch (...) {
                // An exception from a read handler or from onNewFrame ends the
                // capture rather than crashing the thread.
            }
            captureActive_.store(false);
        }

        void issueRead() {
            ctx_.data.async_read_some(
                boost::asio::buffer(staged_),
                [this](const boost::system::error_code& ec, std::size_t n) {
                    if (ec) {
                        // Cancelled (stop requested) or a genuine Data port
                        // error; either way the capture ends here.
                        return;
                    }
                    onBytes(n);
                });
        }

        // Feeds the just-staged bytes through the same framing state machine
        // as FrameReader and dispatches completed frames to the callback.
        void onBytes(std::size_t n) {
            for (std::size_t i = 0; i < n && captureActive_.load(); ++i) {
                buffer_.push_back(staged_[i]);

                if (checkFrameSync()) {
                    headerParsed_ = false;
                }

                if (!headerParsed_) {
                    tryParseHeader();
                }

                if (headerParsed_ &&
                    buffer_.size() >= currentHeader_.totalPacketLen) {
                    extractFrameAndNotify();
                    if (!captureActive_.load()) break;
                }
            }

            if (captureActive_.load()) {
                issueRead();
            }
        }

        // Same magic-word resync logic as FrameReader::checkFrameSync.
        bool checkFrameSync() {
            if (buffer_.size() < sizeof(MAGIC_WORD)) {
                return false;
            }

            const uint8_t* tail =
                buffer_.data() + buffer_.size() - sizeof(MAGIC_WORD);
            if (std::memcmp(tail, MAGIC_WORD, sizeof(MAGIC_WORD)) != 0) {
                return false;
            }

            std::size_t frameStart = buffer_.size() - sizeof(MAGIC_WORD);
            if (frameStart > 0) {
                const auto offset = static_cast<std::ptrdiff_t>(frameStart);
                buffer_.erase(buffer_.begin(), buffer_.begin() + offset);
            }
            return true;
        }

        // Same header parsing as FrameReader::tryParseHeader.
        void tryParseHeader() {
            if (buffer_.size() < sizeof(MAGIC_WORD) + sizeof(FrameHeader)) {
                return;
            }
            std::memcpy(
                &currentHeader_,
                buffer_.data() + sizeof(MAGIC_WORD),
                sizeof(FrameHeader));
            headerParsed_ = true;
        }

        // Same frame slicing as FrameReader::extractFrame, then reports it.
        void extractFrameAndNotify() {
            Streaming::Frame frame;
            frame.header = currentHeader_;

            const auto len = static_cast<std::ptrdiff_t>(currentHeader_.totalPacketLen);
            frame.bytes.assign(buffer_.begin(), buffer_.begin() + len);
            buffer_.erase(buffer_.begin(), buffer_.begin() + len);

            headerParsed_ = false;

            if (onNewFrame_) {
                onNewFrame_(frame);
            }
        }

        Porter::Context& ctx_;
        std::thread captureThread_;
        std::atomic<bool> captureActive_{false};
        FrameCallback onNewFrame_;

        std::vector<uint8_t> buffer_;
        std::array<uint8_t, 256> staged_{};
        bool headerParsed_ = false;
        FrameHeader currentHeader_{};
    };

}