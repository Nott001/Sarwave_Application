#include "MMWave/Streaming/ConstantCapture.hpp"

#include <algorithm>
#include <utility>

#include "MMWave/Common/Exceptions.hpp"

namespace Sarwave::Core::MMWave::Streaming {

using Exceptions::FrameMalformedException;
using Exceptions::FrameReaderMalformedException;

ConstantCapture::ConstantCapture(Porting::Context& ctx) : m_reader(ctx) {
}

ConstantCapture::~ConstantCapture() {
    stopConstantCapture();
    // Only reachable when destroyed from inside its own callback.
    if (m_readerThread.joinable()) m_readerThread.detach();
    if (m_workerThread.joinable()) m_workerThread.detach();
}

std::exception_ptr ConstantCapture::error() const {
    std::lock_guard lock(m_gate);
    return m_error;
}

bool ConstantCapture::isCapturing() const {
    std::lock_guard lock(m_gate);
    return m_running;
}

TimeSpan ConstantCapture::elapsed() const {
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    return TimeSpans::from(std::chrono::steady_clock::duration(now - m_clockStart.load()));
}

void ConstantCapture::beginConstantCapture(FrameCallback onNewFrame,
                                           MalformedCallback onMalformed) {
    std::lock_guard lock(m_gate);

    // Checks the threads' own completion, so a capture that has been told to stop but hasn't
    // finished yet can't overlap a new one on the same, not-thread-safe FrameReader.
    if (m_running) throw InvalidOperationException("ConstantCapture is already capturing frames");
    joinFinishedThreads();

    m_onNewFrame = std::move(onNewFrame);
    m_onMalformed = std::move(onMalformed);
    m_onDropped = onDropped;
    m_onCompleted = onCompleted;
    m_capacity = std::max<std::size_t>(queueCapacity, 1);
    m_error = nullptr;
    m_reader.reset();
    m_skippedFrames = 0;
    m_droppedFrames = 0;
    {
        std::lock_guard queueLock(m_queueGate);
        m_queue.clear();
        m_readerDone = false;
        m_pendingDropped = 0;
    }
    m_clockStart = std::chrono::steady_clock::now().time_since_epoch().count();

    m_stop = std::stop_source();
    m_running = true;
    m_activeThreads = 2;
    m_workerThread = std::thread([this, stop = m_stop] { workerLoop(stop.get_token()); });
    m_readerThread = std::thread([this, token = m_stop.get_token()] { readerLoop(token); });
}

void ConstantCapture::stopConstantCapture(std::optional<std::chrono::milliseconds> waitTimeout) {
    std::unique_lock lock(m_gate);
    if (!m_running) {
        joinFinishedThreads();
        return;
    }

    m_stop.request_stop();

    // Stop called from inside a callback: can't wait for ourselves.
    if (m_callbackThreadId.load() == std::this_thread::get_id()) return;

    const auto finished = [this] { return !m_running; };
    if (waitTimeout) {
        if (!m_finished.wait_for(lock, *waitTimeout, finished)) throw OperationCanceledException();
    } else {
        m_finished.wait(lock, finished);
    }
    joinFinishedThreads();
}

void ConstantCapture::waitForCompletion() {
    std::unique_lock lock(m_gate);
    m_finished.wait(lock, [this] { return !m_running; });
    joinFinishedThreads();
}

void ConstantCapture::joinFinishedThreads() {
    for (std::thread* thread : {&m_readerThread, &m_workerThread}) {
        if (thread->joinable() && thread->get_id() != std::this_thread::get_id()) thread->join();
    }
}

void ConstantCapture::readerLoop(std::stop_token stop) {
    std::exception_ptr failure;
    try {
        while (!stop.stop_requested()) {
            std::optional<Frame> frame;
            try {
                frame = m_reader.readNextFrame(stop);
            } catch (const FrameReaderMalformedException&) {
                // The reader has already moved past the damaged packet and counted it.
                m_skippedFrames = m_reader.skippedFrames();
                if (m_onMalformed) enqueue({std::nullopt, std::current_exception(), elapsed()});
                continue;
            }

            if (!frame) break;  // the data port closed

            const TimeSpan timestamp = elapsed();
            enqueue({std::move(frame), nullptr, timestamp});
        }
    } catch (const OperationCanceledException&) {
        // Normal shutdown -- unless nobody asked to stop.
        if (!stop.stop_requested()) failure = std::current_exception();
    } catch (...) {
        // A failing read ends the capture instead of killing the process. The error is kept so
        // it isn't lost; the worker still delivers what was read before it.
        failure = std::current_exception();
    }

    {
        std::lock_guard lock(m_queueGate);
        m_readerDone = true;
    }
    m_queueChanged.notify_all();
    finishThread(failure);
}

void ConstantCapture::enqueue(Item item) {
    {
        std::lock_guard lock(m_queueGate);
        if (m_queue.size() >= m_capacity) {
            // Drop the oldest frame rather than an error notice: the notices are what tell a
            // corrupted link apart, and there are few of them.
            auto victim =
                std::ranges::find_if(m_queue, [](const Item& i) { return i.frame.has_value(); });
            if (victim == m_queue.end()) victim = m_queue.begin();
            if (victim->frame) {
                ++m_pendingDropped;
                ++m_droppedFrames;
            }
            m_queue.erase(victim);
        }
        m_queue.push_back(std::move(item));
    }
    m_queueChanged.notify_one();
}

void ConstantCapture::workerLoop(std::stop_token stop) {
    m_callbackThreadId.store(std::this_thread::get_id());
    std::exception_ptr failure;
    try {
        while (true) {
            Item item;
            int dropped = 0;
            {
                std::unique_lock lock(m_queueGate);
                m_queueChanged.wait(lock, stop,
                                    [this] { return !m_queue.empty() || m_readerDone; });
                // Stopping: whatever is still queued is disposed unprocessed.
                if (stop.stop_requested() || m_queue.empty()) break;

                item = std::move(m_queue.front());
                m_queue.pop_front();
                dropped = std::exchange(m_pendingDropped, 0);
            }

            if (dropped > 0) notifyDropped(dropped, item.timestamp);

            if (!item.frame) {
                notifyMalformed(item.malformed, item.timestamp);
                continue;
            }

            try {
                notifyFrame(*item.frame, item.timestamp);
            } catch (const FrameMalformedException&) {
                if (!m_onMalformed) throw;
                notifyMalformed(std::current_exception(), item.timestamp);
            }
            item.frame->dispose();
        }
    } catch (...) {
        // A callback that throws ends the capture instead of killing the process.
        failure = std::current_exception();
    }

    if (failure) m_stop.request_stop();  // also ends the reader's pending read
    m_callbackThreadId.store(std::thread::id());
    finishThread(failure);
}

void ConstantCapture::finishThread(std::exception_ptr failure) {
    CompletedCallback completed;
    std::exception_ptr error;
    {
        std::lock_guard lock(m_gate);
        if (failure && !m_error) m_error = failure;
        if (--m_activeThreads > 0) return;

        {
            std::lock_guard queueLock(m_queueGate);
            m_queue.clear();
        }
        m_running = false;
        completed = m_onCompleted;
        error = m_error;
        m_finished.notify_all();
    }

    if (completed) {
        try {
            completed(error);
        } catch (...) {
            // Nothing is left to report it to.
        }
    }
}

void ConstantCapture::notifyFrame(const Frame& frame, TimeSpan timestamp) {
    if (m_onNewFrame) m_onNewFrame(frame, timestamp);
}

void ConstantCapture::notifyMalformed(const std::exception_ptr& exception, TimeSpan timestamp) {
    if (!m_onMalformed) return;
    try {
        std::rethrow_exception(exception);
    } catch (const FrameMalformedException& malformed) {
        m_onMalformed(malformed, timestamp);
    }
}

void ConstantCapture::notifyDropped(int count, TimeSpan timestamp) {
    if (m_onDropped) m_onDropped(count, timestamp);
}

}  // namespace Sarwave::Core::MMWave::Streaming
