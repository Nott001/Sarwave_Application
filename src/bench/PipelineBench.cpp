// Times the live app's per-frame work on a recording, stage by stage, so a change can be
// measured on the machine it runs on (the Raspberry Pi) instead of guessed at:
//
//   reader   FrameReader reassembling frames from the byte stream, as the reader thread does
//   worker   everything the capture's worker thread does per frame in LiveRpiApp, also timed
//            step by step: record the frame, ask the IMU gate, unpack and track it (track), and
//            copy what the dashboard draws (snapshot)
//   paint    drawing that snapshot the way RadarDataOverlay does, on the GUI thread
//
// The frame period is 55 ms. A stage that needs a large part of that on the Pi is the one to work
// on; one that needs a fraction of a millisecond is not.
//
// Usage: SarwaveBench <recording.swrec> [--imu <session.swimu>] [--passes N]
//                     [--size WxH] [--no-paint]
// A busy test recording: python3 tools/generate_recording.py busy.swrec --busy --frames 3000

#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "DevEnv/Gui/ImuFrames.hpp"
#include "Gui/Models/RadarData.hpp"
#include "Gui/ViewModels/Radar/RadarGridViewModel.hpp"
#include "Gui/Views/Radar/RadarDataOverlay.hpp"
#include "Imu/Gating/MotionGate.hpp"
#include "Imu/Recording/ImuRecording.hpp"
#include "MMWave/Exceptions/FrameMalformedException.hpp"
#include "MMWave/Recording/FrameRecorder.hpp"
#include "MMWave/Recording/FrameRecording.hpp"
#include "MMWave/Streaming/FrameReader.hpp"

namespace {

using namespace Sarwave::Core;
using Clock = std::chrono::steady_clock;
using Sarwave::DevEnv::Gui::ImuFrames;

/// How long point clouds stay on the radar grid in the live app (RunStuff's PointRetention).
const MMWave::TimeSpan PointRetention = MMWave::TimeSpans::fromSeconds(1);

/// The size of each read the reader thread gets from the port, roughly: what arrives in a few
/// milliseconds at 921600 baud.
constexpr std::size_t ReadChunk = 512;

struct Stats {
    std::vector<double> micros;

    void add(Clock::duration d) {
        micros.push_back(std::chrono::duration<double, std::micro>(d).count());
    }

    void print(const char* name) {
        if (micros.empty()) return;
        std::sort(micros.begin(), micros.end());
        double sum = 0;
        for (double m : micros) sum += m;
        const auto at = [&](double q) {
            return micros[std::min(micros.size() - 1, static_cast<std::size_t>(q * micros.size()))];
        };
        std::printf("%-8s mean %8.1f us   p50 %8.1f   p99 %8.1f   max %8.1f   (%zu frames)\n", name,
                    sum / static_cast<double>(micros.size()), at(0.50), at(0.99), micros.back(),
                    micros.size());
    }
};

[[noreturn]] void usage() {
    std::cerr << "Usage: SarwaveBench <recording.swrec> [--imu <session.swimu>] [--passes N] "
                 "[--size WxH] [--no-paint]\n";
    std::exit(2);
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) usage();
    std::filesystem::path recordingPath = argv[1];
    std::optional<std::filesystem::path> imuPath;
    int passes = 3;
    int width = 800;
    int height = 480;
    bool paint = true;

    for (int i = 2; i < argc; i++) {
        const std::string arg = argv[i];
        if (arg == "--imu" && i + 1 < argc) {
            imuPath = argv[++i];
        } else if (arg == "--passes" && i + 1 < argc) {
            passes = std::max(1, std::stoi(argv[++i]));
        } else if (arg == "--size" && i + 1 < argc) {
            if (std::sscanf(argv[++i], "%dx%d", &width, &height) != 2) usage();
        } else if (arg == "--no-paint") {
            paint = false;
        } else {
            usage();
        }
    }

    // Fonts and QPainter need a QGuiApplication; nothing is shown.
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "offscreen");
    QGuiApplication app(argc, argv);

    const MMWave::Recording::FrameRecording recording =
        MMWave::Recording::FrameRecording::load(recordingPath);
    const auto frames = recording.frames();
    std::vector<std::uint8_t> stream;
    for (const auto& frame : frames) {
        const auto bytes = frame.bytes();
        stream.insert(stream.end(), bytes.begin(), bytes.end());
    }

    std::optional<Imu::Recording::ImuRecording> imuRecording;
    if (imuPath) imuRecording = Imu::Recording::ImuRecording::load(*imuPath);

    std::printf("%s: %d frames, %zu bytes; IMU %s; %d pass(es)\n",
                recordingPath.filename().string().c_str(), recording.count(), stream.size(),
                imuRecording ? "on" : "off", passes);

    Stats reader;
    Stats record;
    Stats imu;
    Stats track;
    Stats snapshot;
    Stats worker;
    Stats painter;
    int malformed = 0;
    const std::filesystem::path sink =
        std::filesystem::temp_directory_path() / "sarwave_bench.swrec";

    for (int pass = 0; pass < passes; pass++) {
        // Reader: the byte stream in serial-sized reads, one timing per frame produced.
        {
            MMWave::Streaming::FrameReader frameReader;
            MMWave::Streaming::Frame frame;
            auto start = Clock::now();
            for (std::size_t at = 0; at < stream.size(); at += ReadChunk) {
                frameReader.append(
                    std::span(stream).subspan(at, std::min(ReadChunk, stream.size() - at)));
                while (frameReader.tryExtractFrame(frame)) {
                    const auto now = Clock::now();
                    reader.add(now - start);
                    start = now;
                }
            }
        }

        // Worker (+ paint): a fresh pipeline per pass, as a new session would have. The
        // dashboard's view models are left out so each step can be timed on its own; what they
        // do per frame is the snapshot below (MinimalGuiViewModel's status copy is two bools).
        Gui::Models::RadarData data(PointRetention);
        MMWave::Recording::FrameRecorder recorder(sink);

        std::unique_ptr<Imu::Timeline::ImuTimeline> timeline;
        std::unique_ptr<Imu::Gating::MotionGate> gate;
        if (imuRecording) {
            timeline = imuRecording->toTimeline(ImuFrames::options().mounting,
                                                ImuFrames::options().processing);
            gate = std::make_unique<Imu::Gating::MotionGate>(*timeline, ImuFrames::options().gate);
        }

        Gui::Views::Radar::RadarDataOverlay overlay;
        Gui::Views::Radar::RadarGeometry geometry;
        geometry.maxRange = 12;  // RadarGridViewModel's default
        geometry.origin = QPointF(width / 2.0, height - 10.0);
        geometry.scale = (height - 20.0) / geometry.maxRange;
        overlay.setProperty("geometry", QVariant::fromValue(geometry));
        QImage image(width, height, QImage::Format_ARGB32_Premultiplied);

        for (std::size_t i = 0; i < frames.size(); i++) {
            const MMWave::TimeSpan timestamp = recording.timestamps()[i];
            const auto t0 = Clock::now();
            recorder.record(frames[i], timestamp);
            const auto t1 = Clock::now();
            std::optional<Imu::Gating::ImuFrameState> state;
            if (gate) state = gate->evaluate(timestamp);
            const auto t2 = Clock::now();
            try {
                ImuFrames::apply(data, frames[i], timestamp, state);
            } catch (const MMWave::Exceptions::FrameMalformedException&) {
                malformed++;
                data.skipFrame(timestamp);
            }
            const auto t3 = Clock::now();
            auto shot = std::make_shared<const Gui::ViewModels::Radar::RadarSnapshot>(
                Gui::ViewModels::Radar::RadarGridViewModel::createSnapshot(data));
            const auto t4 = Clock::now();

            record.add(t1 - t0);
            imu.add(t2 - t1);
            track.add(t3 - t2);
            snapshot.add(t4 - t3);
            worker.add(t4 - t0);

            if (paint) {
                const auto paintStart = Clock::now();
                overlay.setProperty("snapshot", QVariant::fromValue(std::move(shot)));
                image.fill(Qt::transparent);
                QPainter p(&image);
                overlay.paint(&p);
                p.end();
                painter.add(Clock::now() - paintStart);
            }
        }
    }

    std::error_code ignored;
    std::filesystem::remove(sink, ignored);

    reader.print("reader");
    std::printf("\n");
    record.print("record");
    imu.print("imu");
    track.print("track");
    snapshot.print("snapshot");
    worker.print("worker");
    std::printf("\n");
    painter.print("paint");
    if (malformed > 0) std::printf("(%d malformed frames)\n", malformed);
    std::printf("budget  %8.1f us per frame at 55 ms\n", 55'000.0);
    return 0;
}
