// What the IMU read while the device was held still, for the checks in
// docs/imu_mounting_checks.md. The C++ twin of the C# repo's tools/imu_check.cs; prints the same,
// but for the calibration line, which is in C++.
//
//   SarwaveImuCheck                      the last live session's IMU recording
//   SarwaveImuCheck a.swimu b.swimu      several, averaged (e.g. the table, both ways round)

#include <cmath>
#include <cstdio>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "Imu/Fusion/Fusion.hpp"
#include "Imu/Recording/ImuRecording.hpp"
#include "Integration/PeopleTracking/ImuFrames.hpp"
#include "MMWave/Common/Exceptions.hpp"
#include "ToolSupport.hpp"

namespace {

using namespace Sarwave::Core;
using MMWave::Vector3;
using SarwaveTools::align;
using SarwaveTools::signedFixed;

/// MathF.Round(value, 2) + 0f: rounded first (to even), and + 0 turns −0 into 0, so a value that
/// rounds to zero prints without a minus sign.
float r(float value) {
    return std::nearbyint(value * 100.0f) / 100.0f + 0.0f;
}

std::string upAxis(Vector3 g) {
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

Vector3 abs(Vector3 v) {
    return {std::abs(v.x), std::abs(v.y), std::abs(v.z)};
}

/// {value:F4}
std::string f4(float value) {
    char text[32];
    std::snprintf(text, sizeof text, "%.4f", static_cast<double>(value));
    return text;
}

int run(const std::vector<std::string>& args) {
    std::vector<std::string> paths = args;
    if (paths.empty()) {
        paths.push_back(
            (SarwaveTools::sessionFolder() /
             (std::string("default_session") + Imu::Recording::ImuRecording::SidecarExtension))
                .string());
    }
    Vector3 upSum;

    for (const std::string& path : paths) {
        const auto recording = Imu::Recording::ImuRecording::load(path);
        if (recording.count() < 400) {
            std::cerr << path << ": only " << recording.count()
                      << " samples; hold still for at least 10 s." << std::endl;
            return 1;
        }

        // The middle half, away from pressing start and stop.
        const auto still = recording.samples().subspan(
            static_cast<std::size_t>(recording.count() / 4),
            static_cast<std::size_t>(recording.count() * 3 / 4 - recording.count() / 4));
        Vector3 mean;
        for (const auto& sample : still) mean += sample.acceleration;
        mean = mean / static_cast<float>(still.size());

        Vector3 spread;
        for (const auto& sample : still) spread += abs(sample.acceleration - mean);
        spread = spread / static_cast<float>(still.size());

        std::cout << path << "\n";
        std::cout << "  IMU axes, g:  X " << align(signedFixed(r(mean.x), 2), 6) << "   Y "
                  << align(signedFixed(r(mean.y), 2), 6) << "   Z "
                  << align(signedFixed(r(mean.z), 2), 6) << "\n";
        std::cout << "  pointing up:  " << upAxis(mean) << "\n";
        if (std::abs(mean.length() - 1) > 0.05f || spread.length() > 0.02f)
            std::cout << "  ! the device was not still: record this one again\n";

        upSum += MMWave::normalize(
            Sarwave::Integration::PeopleTracking::ImuFrames::handheldMounting().toRadar(mean));
    }

    const Vector3 up = MMWave::normalize(upSum);
    const auto tilt = Imu::Fusion::TiltCorrection::fromUp(up);
    std::cout << "\n";
    std::cout << "With the current mounting: pitch " << signedFixed(r(tilt.pitchDegrees()), 2)
              << "°, roll " << signedFixed(r(tilt.rollDegrees()), 2) << "°\n";
    // For ImuFrames::handheldMounting(); the C# tool prints the same numbers in C# syntax.
    std::cout << "Level calibration:  .withLevelCalibration({" << f4(up.x) << "f, " << f4(up.y)
              << "f, " << f4(up.z) << "f})\n";
    return 0;
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        return run(std::vector<std::string>(argv + 1, argv + argc));
    } catch (...) {
        Sarwave::Core::failUnhandled(std::current_exception());
    }
}
