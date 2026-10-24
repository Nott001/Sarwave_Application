#include "ImuMonitor.hpp"

#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <exception>
#include <iostream>
#include <mutex>
#include <thread>

#include "Imu/ImuSession.hpp"
#include "Integration/PeopleTracking/ImuFrames.hpp"
#include "MMWave/Common/DotNetFormat.hpp"
#include "MMWave/Common/Exceptions.hpp"

#ifndef _WIN32
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace {

using namespace Sarwave::Core;
using MMWave::Vector3;
using MMWave::DotNet::formatFixed;

/// The radar's frame period in the Sarwave cfgs (frameCfg 60 ms): the gate is asked this often.
constexpr std::chrono::milliseconds FramePeriod{60};

constexpr int Lines = 7;

std::atomic<bool> g_stop = false;

extern "C" void onInterrupt(int) {
    g_stop = true;
}

/// Always signed, `decimals` places, padded to `decimals` + 4 characters; rounded first (to
/// even, as MathF.Round), and + 0 turns −0 into 0, so zero prints as +0.
std::string signedText(float value, int decimals) {
    const float scale = std::pow(10.0f, static_cast<float>(decimals));
    const float rounded = std::nearbyint(value * scale) / scale + 0.0f;
    std::string text =
        (rounded < 0 ? "-" : "+") + formatFixed(std::abs(rounded), decimals, decimals);
    const std::size_t width = static_cast<std::size_t>(decimals) + 4;
    if (text.size() < width) text.insert(0, width - text.size(), ' ');
    return text;
}

/// {value,width:0.0...}: right-aligned in `width` characters.
std::string padded(float value, int decimals, std::size_t width) {
    std::string text = formatFixed(value, decimals, decimals);
    if (text.size() < width) text.insert(0, width - text.size(), ' ');
    return text;
}

std::string upAxis(Vector3 g) {
    if (g == Vector3::zero()) return "-";
    const float ax = std::abs(g.x);
    const float ay = std::abs(g.y);
    const float az = std::abs(g.z);
    float value = g.z;
    char axis = 'Z';
    if (ax >= ay && ax >= az) {
        value = g.x;
        axis = 'X';
    } else if (ay >= az) {
        value = g.y;
        axis = 'Y';
    }
    return std::string(value >= 0 ? "+" : "-") + axis;
}

std::string verdict(const Imu::Gating::ImuFrameState& state) {
    switch (state.state) {
        case Imu::Gating::MotionGateState::Steady:
            return "STEADY";
        case Imu::Gating::MotionGateState::Settling:
            return "SETTLING " +
                   formatFixed(std::chrono::duration<double>(state.settleRemaining).count(), 1, 1) +
                   " s";
        case Imu::Gating::MotionGateState::Moving:
            return "MOVING";
        default:
            return "NO DATA";
    }
}

std::array<std::string, Lines> describe(Imu::ImuSession& imu,
                                        const Imu::Gating::ImuFrameState& state, Vector3 raw) {
    const auto latest = imu.timeline().tryGetLatest();
    const std::string temperature =
        latest ? formatFixed(latest->temperature, 1, 1) + " °C" : std::string("-");

    // The processor belongs to the sampler thread; these two are read only for display, and a
    // reading torn by a concurrent update only spoils one redraw.
    const Vector3 bias = imu.processor().gyroBias();
    const std::string still = imu.processor().isStill() ? "learning it: the device is set down"
                                                        : "kept: learned only while set down";

    const std::string tilt = state.tilt.isValid()
                                 ? "pitch " + signedText(state.tilt.pitchDegrees(), 1) +
                                       "°   roll " + signedText(state.tilt.rollDegrees(), 1) +
                                       "°   (nose up = +pitch, right side down = +roll)"
                                 : "-";

    std::string gate = verdict(state);
    if (gate.size() < 15) gate.append(15 - gate.size(), ' ');

    const double lookBackMs =
        std::chrono::duration<double, std::milli>(imu.gate().settings().lookBack).count();

    return {
        "  Raw, IMU axes   X " + signedText(raw.x, 2) + "   Y " + signedText(raw.y, 2) + "   Z " +
            signedText(raw.z, 2) + " g   pointing up: " + upAxis(raw),
        "  Tilt            " + tilt,
        "  Turning         " + padded(state.window.meanAngularRate, 1, 5) + " °/s average, " +
            padded(state.window.peakAngularRate, 1, 5) + " °/s peak (last " +
            formatFixed(lookBackMs, 0, 0) + " ms)",
        "  Pushed          " + padded(state.window.meanLinearAcceleration, 3, 5) + " g average, " +
            padded(state.window.peakLinearAcceleration, 3, 5) + " g peak",
        "  Gate            " + gate + " frames accepted " +
            std::to_string(imu.gate().acceptedFrames()) + ", refused " +
            std::to_string(imu.gate().rejectedFrames()),
        "  Gyro bias       X " + signedText(bias.x, 2) + "   Y " + signedText(bias.y, 2) + "   Z " +
            signedText(bias.z, 2) + " °/s   (" + still + ")",
        "  Device          " + temperature + "   " + std::to_string(imu.sampler().sampleCount()) +
            " samples, " + std::to_string(imu.sampler().overflows()) + " FIFO overflows",
    };
}

/// Whether stdout is a terminal the block can be redrawn in.
bool canRedraw() {
#ifdef _WIN32
    return false;
#else
    return isatty(STDOUT_FILENO) != 0;
#endif
}

/// Console.WindowWidth.
int terminalWidth() {
#ifndef _WIN32
    winsize size{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0 && size.ws_col > 0) return size.ws_col;
#endif
    return 80;
}

/// The line cut or padded to exactly `width` columns.
std::string fit(const std::string& line, std::size_t width) {
    std::string out;
    std::size_t count = 0;
    for (const char c : line) {
        const bool starts = (static_cast<unsigned char>(c) & 0xC0) != 0x80;
        if (starts && count == width) break;
        if (starts) ++count;
        out += c;
    }
    if (count < width) out.append(width - count, ' ');
    return out;
}

}  // namespace

int ImuMonitor::run(const std::optional<std::string>& recordingPath) {
    using Sarwave::Integration::PeopleTracking::ImuFrames;

    std::string reason;
    std::unique_ptr<Imu::ImuSession> imu = Imu::ImuSession::tryOpen(ImuFrames::options(), reason);
    if (!imu) {
        std::cerr << "No IMU: " << reason << std::endl;
        return 1;
    }

    std::signal(SIGINT, onInterrupt);

    // Raw readings in IMU axes, summed on the sampler thread and taken by each redraw.
    std::mutex rawLock;
    Vector3 rawSum;
    int rawCount = 0;

    const auto started = std::chrono::steady_clock::now();
    const auto elapsed = [started] {
        return std::chrono::duration_cast<MMWave::TimeSpan>(std::chrono::steady_clock::now() -
                                                            started);
    };
    try {
        imu->start(
            elapsed,
            recordingPath ? std::optional<std::filesystem::path>(*recordingPath) : std::nullopt,
            [&](std::span<const Imu::ImuSample> samples) {
                std::lock_guard lock(rawLock);
                for (const auto& sample : samples) rawSum = rawSum + sample.acceleration;
                rawCount += static_cast<int>(samples.size());
            });
    } catch (const IOException& ex) {
        std::cerr << "Cannot record to " << *recordingPath << ": " << ex.what() << std::endl;
        return 1;
    }

    std::cout << "Sarwave IMU monitor: Ctrl+C to stop\n";
    std::cout << (recordingPath ? "Recording raw samples to " + *recordingPath : "Not recording.")
              << "\n\n";
    const bool redraw = canRedraw();
    if (redraw) {
        std::cout << "\x1b[?25l";  // cursor hidden
        for (int i = 0; i < Lines; ++i) std::cout << "\n";
    }
    std::cout << std::flush;

    Vector3 raw;
    MMWave::TimeSpan nextPrint{};
    while (!g_stop && imu->sampler().isRunning()) {
        std::this_thread::sleep_for(FramePeriod);
        const auto state = imu->evaluate(elapsed());

        {
            std::lock_guard lock(rawLock);
            if (rawCount > 0) {
                const Vector3 mean = rawSum / static_cast<float>(rawCount);
                // Vector3.Lerp(raw, mean, 0.3f)
                raw = raw == Vector3::zero() ? mean : raw * (1.0f - 0.3f) + mean * 0.3f;
            }
            rawSum = Vector3::zero();
            rawCount = 0;
        }

        const auto block = describe(*imu, state, raw);
        if (redraw) {
            // The block is rewritten in place: back up over it, each line fitted to the width.
            const auto width = static_cast<std::size_t>(std::max(1, terminalWidth() - 1));
            std::cout << "\x1b[" << Lines << "A";
            for (const auto& line : block) std::cout << "\r" << fit(line, width) << "\n";
            std::cout << std::flush;
        } else if (elapsed() >= nextPrint) {
            // Piped to a file: a block every second instead.
            nextPrint = elapsed() + MMWave::TimeSpans::fromSeconds(1);
            for (const auto& line : block) std::cout << line << "\n";
            std::cout << std::endl;
        }
    }
    if (redraw) std::cout << "\x1b[?25h" << std::flush;  // cursor shown

    imu->dispose();
    if (const auto error = imu->sampler().error()) {
        try {
            std::rethrow_exception(error);
        } catch (const std::exception& ex) {
            std::cerr << "The IMU stopped: " << ex.what() << std::endl;
        } catch (...) {
            std::cerr << "The IMU stopped." << std::endl;
        }
        return 1;
    }
    return 0;
}
