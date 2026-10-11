#include "MMWave/Streaming/ConstantCapture.hpp"

#include "MMWave/Common/Exceptions.hpp"

namespace Sarwave::Core::MMWave::Streaming {

ConstantCapture::ConstantCapture(Porting::Context& ctx) : m_reader(ctx) {
}

ConstantCapture::~ConstantCapture() {
    stopConstantCapture();
    if (m_thread.joinable()) {
        // Only reachable when destroyed from inside its own callback.
        m_thread.detach();
    }
}

std::exception_ptr ConstantCapture::error() const {
    std::lock_guard lock(m_gate);
    return m_error;
}

bool ConstantCapture::isCapturing() const {
    std::lock_guard lock(m_gate);
    return m_running;
}

void ConstantCapture::beginConstantCapture(FrameCallback onNewFrame) {
    std::lock_guard lock(m_gate);

    // Checks the loop's own completion, so a capture that has been told to stop but hasn't
    // finished yet can't overlap a new one on the same, not-thread-safe FrameReader.
    if (m_running) throw InvalidOperationException("ConstantCapture is already capturing frames");
    joinFinishedThread();

    m_onNewFrame = std::move(onNewFrame);
    m_error = nullptr;
    m_reader.reset();

    m_stop = std::stop_source();
    m_running = true;
    m_thread = std::thread([this, token = m_stop.get_token()] { captureLoop(token); });
}

void ConstantCapture::stopConstantCapture(std::optional<std::chrono::milliseconds> waitTimeout) {
    std::unique_lock lock(m_gate);
    if (!m_running) {
        joinFinishedThread();
        return;
    }

    m_stop.request_stop();

    // Stop called from inside the frame callback: can't wait for ourselves.
    if (m_callbackThreadId.load() == std::this_thread::get_id()) return;

    const auto finished = [this] { return !m_running; };
    if (waitTimeout) {
        if (!m_finished.wait_for(lock, *waitTimeout, finished)) throw OperationCanceledException();
    } else {
        m_finished.wait(lock, finished);
    }
    joinFinishedThread();
}

void ConstantCapture::joinFinishedThread() {
    if (m_thread.joinable() && m_thread.get_id() != std::this_thread::get_id()) m_thread.join();
}

void ConstantCapture::captureLoop(std::stop_token stop) {
    std::exception_ptr failure;
    try {
        while (!stop.stop_requested()) {
            std::optional<Frame> frame = m_reader.readNextFrame(stop);
            if (!frame) break;

            notifyFrame(*frame);
            frame->dispose();
        }
    } catch (const OperationCanceledException&) {
        // Normal shutdown -- unless nobody asked to stop.
        if (!stop.stop_requested()) failure = std::current_exception();
    } catch (...) {
        // A failing read or callback ends the capture instead of killing the process. The
        // error is kept so it isn't lost.
        failure = std::current_exception();
    }

    std::lock_guard lock(m_gate);
    m_error = failure;
    m_running = false;
    m_finished.notify_all();
}

void ConstantCapture::notifyFrame(const Frame& frame) {
    m_callbackThreadId.store(std::this_thread::get_id());
    try {
        if (m_onNewFrame) m_onNewFrame(frame);
    } catch (...) {
        m_callbackThreadId.store(std::thread::id());
        throw;
    }
    m_callbackThreadId.store(std::thread::id());
}

}  // namespace Sarwave::Core::MMWave::Streaming
