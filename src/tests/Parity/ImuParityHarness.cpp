// Prints what the IMU library makes of a recording in a fixed text layout. Its C# twin
// (csharp-imu/, built against the original Sarwave.Core.Imu) prints the same layout; diffing the
// two proves the port fuses, gates and records identically.
//
//   ImuParityHarness process <file.swimu> <every>         processed state of every n-th sample
//   ImuParityHarness track <file.swimu> <every>           pose of every n-th sample (handheld)
//   ImuParityHarness gate <file.swimu> <framePeriodMs>    the gate's verdict on a frame stream
//   ImuParityHarness rewrite <in.swimu> <out.swimu>       ImuRecorder round trip
//   ImuParityHarness mount                                ImuMounting.FromAxes for every triple

#include <iostream>
#include <string>
#include <vector>

#include "Imu/Fusion/Fusion.hpp"
#include "Imu/Fusion/ImuProcessor.hpp"
#include "Imu/Gating/MotionGate.hpp"
#include "Imu/Recording/ImuRecording.hpp"
#include "MMWave/Common/DotNetFormat.hpp"
#include "MMWave/Common/Exceptions.hpp"

using namespace Sarwave::Core;
using namespace Sarwave::Core::Imu;

namespace {

std::string F(float v) {
    return MMWave::DotNet::formatSingle(v);
}

std::string V(Vector3 v) {
    return F(v.x) + "," + F(v.y) + "," + F(v.z);
}

std::string Q(Quaternion q) {
    return F(q.x) + "," + F(q.y) + "," + F(q.z) + "," + F(q.w);
}

int cmdProcess(const std::string& path, int every) {
    const auto recording = Recording::ImuRecording::load(path);
    Fusion::ImuProcessor processor(Fusion::ImuMounting::identity());
    int i = 0;
    for (const ImuSample& sample : recording.samples()) {
        const ImuState s = processor.process(sample);
        if (i++ % every != 0) continue;
        std::cout << s.timestamp.count() << " a=" << V(s.acceleration) << " w=" << V(s.angularRate)
                  << " up=" << V(s.up) << " bias=" << V(processor.gyroBias())
                  << " still=" << (processor.isStill() ? "True" : "False") << "\n";
    }
    std::cout << "count=" << recording.count()
              << " truncated=" << (recording.isTruncated() ? "True" : "False")
              << " rate=" << F(recording.sampleRate()) << "\n";
    return 0;
}

int cmdTrack(const std::string& path, int every) {
    // The handheld's mounting, radar offset included, so the pose and the offset are compared.
    const auto recording = Recording::ImuRecording::load(path);
    auto mounting = Fusion::ImuMounting::fromAxes(
        Fusion::ImuAxis::PositiveZ, Fusion::ImuAxis::PositiveX, Fusion::ImuAxis::PositiveY);
    mounting.radarOffset = {0.089f, 0.017f, 0.035f};
    Fusion::ImuProcessor processor(mounting);
    int i = 0;
    for (const ImuSample& sample : recording.samples()) {
        const ImuState s = processor.process(sample);
        if (i++ % every != 0) continue;
        std::cout << s.timestamp.count() << " up=" << V(s.up) << " o=" << Q(s.orientation)
                  << " v=" << V(s.velocity) << " p=" << V(s.position) << "\n";
    }
    return 0;
}

int cmdGate(const std::string& path, double framePeriodMs) {
    const auto recording = Recording::ImuRecording::load(path);
    const auto timeline = recording.toTimeline(Fusion::ImuMounting::identity());
    Gating::MotionGate gate(*timeline);
    const TimeSpan first = recording.samples().front().timestamp;
    const TimeSpan last = recording.samples().back().timestamp;
    const TimeSpan period = TimeSpans::fromSeconds(framePeriodMs / 1000.0);
    for (TimeSpan t = first + TimeSpans::fromSeconds(0.2); t <= last; t += period) {
        const auto s = gate.evaluate(t);
        const auto& w = s.window;
        std::cout << t.count() << " " << Gating::toString(s.state)
                  << " accepted=" << (s.accepted ? "True" : "False")
                  << " settle=" << s.settleRemaining.count() << " n=" << w.sampleCount
                  << " peak=" << F(w.peakAngularRate) << "," << F(w.peakLinearAcceleration)
                  << " mean=" << F(w.meanAngularRate) << "," << F(w.meanLinearAcceleration)
                  << " up=" << V(w.meanUp) << " gap=" << w.maxGap.count() << ","
                  << w.trailingGap.count() << " pitch=" << F(s.tilt.pitchDegrees())
                  << " roll=" << F(s.tilt.rollDegrees()) << " q=" << Q(s.tilt.rotation())
                  << " pose="
                  << (s.pose ? std::to_string(s.pose->timestamp.count()) + "," +
                                   Q(s.pose->orientation) + "," + V(s.pose->position)
                             : std::string("none"))
                  << "\n";
    }
    std::cout << "accepted=" << gate.acceptedFrames() << " rejected=" << gate.rejectedFrames()
              << "\n";
    return 0;
}

int cmdRewrite(const std::string& in, const std::string& out) {
    const auto recording = Recording::ImuRecording::load(in);
    Recording::ImuRecorder recorder(out, recording.sampleRate());
    recorder.record(recording.samples());
    std::cout << "samples=" << recorder.sampleCount() << "\n";
    return 0;
}

int cmdMount() {
    using Fusion::ImuAxis;
    for (int a = 0; a < 6; a++)
        for (int b = 0; b < 6; b++)
            for (int c = 0; c < 6; c++) {
                const auto right = static_cast<ImuAxis>(a);
                const auto forward = static_cast<ImuAxis>(b);
                const auto up = static_cast<ImuAxis>(c);
                std::cout << a << b << c << " ";
                try {
                    const auto m = Fusion::ImuMounting::fromAxes(right, forward, up);
                    const auto trimmed = m.withLevelCalibration({0.03f, -0.05f, 1});
                    std::cout << Q(m.rotation) << " x=" << V(m.toRadar(Vector3::unitX()))
                              << " t=" << Q(trimmed.rotation) << "\n";
                } catch (const std::exception& e) {
                    std::cout << "EXCEPTION " << dotNetTypeName(e) << ": " << e.what() << "\n";
                }
            }
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    const std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty()) return 2;
    if (args[0] == "process") return cmdProcess(args[1], std::stoi(args[2]));
    if (args[0] == "track") return cmdTrack(args[1], std::stoi(args[2]));
    if (args[0] == "gate") return cmdGate(args[1], std::stod(args[2]));
    if (args[0] == "rewrite") return cmdRewrite(args[1], args[2]);
    if (args[0] == "mount") return cmdMount();
    return 2;
}
