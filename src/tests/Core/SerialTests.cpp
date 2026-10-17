// End-to-end tests of the serial layer against a fake IWR6843 built from two pseudo-terminals
// (CLI and data port), so Context, sendConfigFile, ConstantCapture and FrameRecorder run exactly
// as they do against the real device. Linux only (openpty).

#ifdef __linux__

#include <poll.h>
#include <pty.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "FrameBuilder.hpp"
#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Exceptions/FrameMalformedException.hpp"
#include "MMWave/Porting/Context.hpp"
#include "MMWave/Recording/FrameRecorder.hpp"
#include "MMWave/Recording/FrameRecording.hpp"
#include "MMWave/Streaming/ConstantCapture.hpp"
#include "TestFramework.hpp"

namespace {

using namespace Sarwave::Core;
using namespace Sarwave::Core::MMWave;
using namespace std::chrono_literals;

struct Pty {
    int master = -1;
    int slave = -1;
    std::string path;

    Pty() {
        char name[256] = {};
        if (openpty(&master, &slave, name, nullptr, nullptr) != 0)
            throw std::runtime_error("openpty failed");
        path = name;
    }
    ~Pty() {
        close(master);
        close(slave);
    }
};

/// Answers CLI commands like the mmWave demo ("<command>\r\nDone\r\nmmwDemo:/>") and, after
/// sensorStart, streams `frames` on the data port with some garbage in between.
class FakeDevice {
   public:
    FakeDevice(Pty& cli, Pty& data, std::vector<std::vector<std::uint8_t>> frames,
               bool answerCommands = true)
        : m_cli(cli), m_data(data), m_frames(std::move(frames)), m_answer(answerCommands) {
        m_thread = std::thread([this] { run(); });
    }
    ~FakeDevice() {
        m_stop = true;
        m_thread.join();
    }

    std::vector<std::string> commands() {
        std::lock_guard lock(m_gate);
        return m_commands;
    }

   private:
    void run() {
        std::string line;
        while (!m_stop) {
            pollfd p{m_cli.master, POLLIN, 0};
            if (poll(&p, 1, 10) <= 0) continue;
            char c;
            if (read(m_cli.master, &c, 1) != 1) continue;
            if (c != '\n') {
                line += c;
                continue;
            }
            {
                std::lock_guard lock(m_gate);
                m_commands.push_back(line);
            }
            if (m_answer) {
                std::string reply = line == "version"
                                        ? "version\r\nmmWave SDK Version: 03.06.02.00\r\nDone\r\n"
                                        : line + "\r\nDone\r\n";
                reply += "mmwDemo:/>";
                (void)!write(m_cli.master, reply.data(), reply.size());
            }
            if (line == "sensorStart") {
                for (const auto& frame : m_frames) {
                    const std::uint8_t garbage[] = {0x11, 0x02, 0x01};
                    (void)!write(m_data.master, garbage, sizeof garbage);
                    (void)!write(m_data.master, frame.data(), frame.size());
                    std::this_thread::sleep_for(5ms);
                }
            }
            line.clear();
        }
    }

    Pty& m_cli;
    Pty& m_data;
    std::vector<std::vector<std::uint8_t>> m_frames;
    bool m_answer;
    std::atomic<bool> m_stop{false};
    std::mutex m_gate;
    std::vector<std::string> m_commands;
    std::thread m_thread;
};

std::vector<std::vector<std::uint8_t>> someFrames(int n) {
    std::vector<std::vector<std::uint8_t>> frames;
    for (int i = 0; i < n; ++i) {
        frames.push_back(SarwaveTest::FrameBuilder()
                             .targets({1})
                             .heights({1}, 1.7f, 0.2f)
                             .cloud(i % 5)
                             .presence(true)
                             .bytes(static_cast<std::uint32_t>(i)));
    }
    return frames;
}

}  // namespace

TEST_CASE("serial: version, cfg upload without sensorStart, capture and recording") {
    Pty cli;
    Pty data;
    const auto frames = someFrames(25);
    FakeDevice device(cli, data, frames);

    auto ctx = Porting::Context::openMmwavePorts(cli.path, data.path);
    CHECK(ctx->validateCliPort());

    std::vector<std::string> reported;
    ctx->sendConfigFile(
        SARWAVE_SOURCE_DIR "/configs/AOP_6m_default.cfg",
        [&](const std::string& line, const std::string& response) {
            reported.push_back(line);
            CHECK(response.find("Done") != std::string::npos);
        },
        true);
    // Every command of the file (CRLF endings, comments and the sensorStart line dropped).
    CHECK_EQ(reported.size(), std::size_t{31});
    CHECK_EQ(reported.front(), "sensorStop");
    CHECK_EQ(reported.back(), "presenceBoundaryBox -3 3 0.5 7.5 0 3");

    std::stringstream file;
    Recording::FrameRecorder recorder(file, true);
    std::mutex gate;
    std::vector<std::vector<std::uint8_t>> received;

    Streaming::ConstantCapture capture(*ctx);
    capture.beginConstantCapture([&](const Streaming::Frame& frame, TimeSpan elapsed) {
        recorder.record(frame, elapsed);
        std::lock_guard lock(gate);
        received.emplace_back(frame.bytes().begin(), frame.bytes().end());
    });
    CHECK_THROWS_AS(capture.beginConstantCapture([](const Streaming::Frame&, TimeSpan) {}),
                    InvalidOperationException);

    const std::string response = ctx->sendCliCommand(Porting::Context::SensorStartCommand, 5s);
    CHECK(response.ends_with("Done"));

    for (int i = 0; i < 400; ++i) {
        {
            std::lock_guard lock(gate);
            if (received.size() >= frames.size()) break;
        }
        std::this_thread::sleep_for(5ms);
    }

    const auto stopStart = std::chrono::steady_clock::now();
    capture.stopConstantCapture(1s);
    CHECK(std::chrono::steady_clock::now() - stopStart < 500ms);
    CHECK(!capture.isCapturing());
    CHECK(!capture.error());

    {
        std::lock_guard lock(gate);
        CHECK(received == frames);
    }
    recorder.dispose();
    std::istringstream in(file.str());
    CHECK_EQ(Recording::FrameRecording::load(in).count(), static_cast<int>(frames.size()));
}

TEST_CASE("serial: a silent CLI times out and a stop interrupts a blocked read") {
    Pty cli;
    Pty data;
    FakeDevice device(cli, data, {}, false);

    auto ctx = Porting::Context::openMmwavePorts(cli.path, data.path);
    const auto before = std::chrono::steady_clock::now();
    CHECK(ctx->sendCliCommand("version", 600ms).empty());
    const auto waited = std::chrono::steady_clock::now() - before;
    CHECK(waited >= 600ms && waited < 1500ms);

    Streaming::ConstantCapture capture(*ctx);
    capture.beginConstantCapture([](const Streaming::Frame&, TimeSpan) {});
    std::this_thread::sleep_for(100ms);
    const auto stopStart = std::chrono::steady_clock::now();
    capture.stopConstantCapture();
    CHECK(std::chrono::steady_clock::now() - stopStart < 500ms);
    CHECK(!capture.error());
}

TEST_CASE("serial: opening a missing port fails with .NET's message and leaves nothing open") {
    try {
        Porting::Context::openMmwavePorts("/dev/ttyDOESNOTEXIST0", "/dev/ttyDOESNOTEXIST1");
        CHECK(false);
    } catch (const IOException& e) {
        CHECK_EQ(std::string(e.what()), "Could not find file '/dev/ttyDOESNOTEXIST0'.");
    }
}

TEST_CASE("serial: sendConfigFile reports a missing file and an unanswered command") {
    Pty cli;
    Pty data;
    FakeDevice device(cli, data, {}, false);
    auto ctx = Porting::Context::openMmwavePorts(cli.path, data.path);
    try {
        ctx->sendConfigFile("/nonexistent.cfg");
        CHECK(false);
    } catch (const FileNotFoundException& e) {
        CHECK_EQ(std::string(e.what()), "Could not open cfg file: /nonexistent.cfg");
    }
}

TEST_CASE("serial: damaged packets are reported in order and the capture carries on") {
    Pty cli;
    Pty data;
    auto frames = someFrames(12);
    // Frame 3 loses 7 bytes from the middle of its target list (as recording D on the Pi did),
    // frame 7 loses its last 40 bytes, so frame 8's magic word arrives inside it.
    frames[3].erase(frames[3].begin() + 80, frames[3].begin() + 87);
    frames[7].resize(frames[7].size() - 40);
    FakeDevice device(cli, data, frames);
    auto ctx = Porting::Context::openMmwavePorts(cli.path, data.path);

    std::mutex gate;
    std::vector<std::string> events;
    std::atomic<bool> completed{false};
    Streaming::ConstantCapture capture(*ctx);
    capture.onCompleted = [&](std::exception_ptr) { completed = true; };
    capture.beginConstantCapture(
        [&](const Streaming::Frame& frame, TimeSpan) {
            std::lock_guard lock(gate);
            events.push_back("frame " + std::to_string(frame.header().frameNumber));
        },
        [&](const Exceptions::FrameMalformedException& ex, TimeSpan) {
            const auto* reader =
                dynamic_cast<const Exceptions::FrameReaderMalformedException*>(&ex);
            std::lock_guard lock(gate);
            events.push_back(std::string("malformed ") +
                             (reader ? Exceptions::toString(reader->cause()) : "?") + " " +
                             std::to_string(ex.frameNumber().value_or(9999)));
        });
    ctx->sendCliCommand(Porting::Context::SensorStartCommand, 5s);

    for (int i = 0; i < 400; ++i) {
        {
            std::lock_guard lock(gate);
            if (!events.empty() && events.back() == "frame 11") break;
        }
        std::this_thread::sleep_for(5ms);
    }
    capture.stopConstantCapture(1s);
    CHECK(completed);
    CHECK(!capture.error());
    CHECK_EQ(capture.skippedFrames(), 2);

    std::lock_guard lock(gate);
    const std::vector<std::string> expected{
        "frame 0", "frame 1", "frame 2",  "malformed TlvChain 3",
        "frame 4", "frame 5", "frame 6",  "malformed Interrupted 7",
        "frame 8", "frame 9", "frame 10", "frame 11"};
    CHECK(events == expected);
}

TEST_CASE("serial: a slow callback no longer starves the reader; overflow is reported apart") {
    Pty cli;
    Pty data;
    const auto frames = someFrames(30);
    FakeDevice device(cli, data, frames);
    auto ctx = Porting::Context::openMmwavePorts(cli.path, data.path);

    std::atomic<int> delivered{0};
    std::atomic<int> dropped{0};
    std::atomic<int> malformed{0};
    Streaming::ConstantCapture capture(*ctx);
    capture.queueCapacity = 4;
    capture.onDropped = [&](int count, TimeSpan) { dropped += count; };
    capture.beginConstantCapture(
        [&](const Streaming::Frame&, TimeSpan) {
            std::this_thread::sleep_for(25ms);  // five times slower than the frames arrive
            delivered++;
        },
        [&](const Exceptions::FrameMalformedException&, TimeSpan) { malformed++; });
    ctx->sendCliCommand(Porting::Context::SensorStartCommand, 5s);

    for (int i = 0; i < 400 && delivered + dropped < 30; ++i) std::this_thread::sleep_for(5ms);
    capture.stopConstantCapture(1s);

    CHECK_EQ(malformed.load(), 0);
    CHECK_EQ(delivered + dropped, 30);
    CHECK(dropped > 0);
    CHECK_EQ(capture.droppedFrames(), dropped.load());
}

TEST_CASE("serial: stopping from inside the callback and a callback that throws") {
    Pty cli;
    Pty data;
    FakeDevice device(cli, data, someFrames(10));
    auto ctx = Porting::Context::openMmwavePorts(cli.path, data.path);

    Streaming::ConstantCapture capture(*ctx);
    std::atomic<int> seen{0};
    capture.beginConstantCapture([&](const Streaming::Frame&, TimeSpan) {
        if (++seen == 3) throw InvalidOperationException("boom");
    });
    ctx->sendCliCommand(Porting::Context::SensorStartCommand, 5s);
    capture.waitForCompletion();
    CHECK(!capture.isCapturing());
    CHECK_EQ(seen.load(), 3);
    REQUIRE(capture.error());
    CHECK_THROWS_AS(std::rethrow_exception(capture.error()), InvalidOperationException);
}

#endif
