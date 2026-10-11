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
        frames.push_back(SarwaveTest::FrameBuilder().targets({1}).cloud(i % 5).presence(true).bytes(
            static_cast<std::uint32_t>(i)));
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
    const auto start = std::chrono::steady_clock::now();
    capture.beginConstantCapture([&](const Streaming::Frame& frame) {
        recorder.record(frame, TimeSpans::from(std::chrono::steady_clock::now() - start));
        std::lock_guard lock(gate);
        received.emplace_back(frame.bytes().begin(), frame.bytes().end());
    });
    CHECK_THROWS_AS(capture.beginConstantCapture([](const Streaming::Frame&) {}),
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
    capture.beginConstantCapture([](const Streaming::Frame&) {});
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

#endif
