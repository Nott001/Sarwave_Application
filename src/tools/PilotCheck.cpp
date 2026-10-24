// How many points a person gives in a pilot recording, per frame and per hold window, for the
// pilot in docs/data_collection_checklist.md. How to record for it and read it:
// docs/pilot_check.md. The C++ twin of the C# repo's tools/pilot_check.cs; prints the same.
//
//   SarwavePilotCheck <session.swrec, or its folder> --at X,Y [options]
//
//   --at X,Y        where the person was, in metres: X to the radar's right, Y ahead (required)
//   --radius R      radius of the person-sized cylinder around --at, in metres (0.5)
//   --zmin, --zmax  height band of the cylinder above the floor, in metres (-0.2, 2.2)
//   --height H      height the radar was held at, in metres (1.2, the cfg's sensorPosition)
//   --window S      hold window length, in seconds (3)
//   --min N         "enough points" for the second row of the tables (3)
//   --each          also list every window

#include <algorithm>
#include <chrono>
#include <cmath>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "Integration/PeopleTracking/Association/PointEntry.hpp"
#include "Integration/PeopleTracking/EntryPackage.hpp"
#include "Integration/PeopleTracking/ImuFrames.hpp"
#include "MMWave/Common/DotNetFormat.hpp"
#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Exceptions/FrameMalformedException.hpp"
#include "MMWave/Recording/FrameRecording.hpp"
#include "ToolSupport.hpp"

namespace {

using namespace Sarwave::Core;
using MMWave::TimeSpan;
using MMWave::Vector3;
using MMWave::DotNet::formatFixed;
using Sarwave::Integration::PeopleTracking::EntryPackage;
using Sarwave::Integration::PeopleTracking::ImuFrames;
using Sarwave::Integration::PeopleTracking::Association::PointEntry;
using SarwaveTools::align;

constexpr int PointCap =
    150;  // the most points the radar sends in a frame (docs/recorded_data.txt)

/// A wrong argument, shown with the usage.
struct UsageError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct Window {
    TimeSpan start{};
    int frames = 0;
    int kept = 0;
    int pointsAt = 0;
    int pointsElsewhere = 0;
};

/// float.Parse(text, CultureInfo.InvariantCulture).
float number(const std::string& text) {
    float value = 0;
    if (!MMWave::DotNet::tryParseSingle(text, value))
        throw UsageError("The input string '" + text + "' was not in a correct format.");
    return value;
}

int usage(const std::string& problem) {
    std::cerr << problem << "\n";
    std::cerr << "Usage: SarwavePilotCheck <session.swrec, or its folder> --at X,Y [--radius 0.5] "
                 "[--zmin -0.2] [--zmax 2.2] [--height 1.2] [--window 3] [--min 3] [--each]\n";
    std::cerr << "How to use it: docs/pilot_check.md" << std::endl;
    return 2;
}

/// {100.0 * count / of:0} %
std::string percent(int count, int of) {
    return of == 0 ? "-" : formatFixed(100.0 * count / of, 0, 0) + " %";
}

std::string median(std::vector<int> values) {
    std::ranges::sort(values);
    return values.empty() ? "-" : std::to_string(values[values.size() / 2]);
}

std::string spread(std::vector<float> values, int decimals) {
    if (values.empty()) return "no points";
    std::ranges::sort(values);
    const auto at = [&](double q) {
        return formatFixed(
            values[static_cast<std::size_t>(q * static_cast<double>(values.size() - 1))], decimals,
            decimals);
    };
    return at(0.1) + " / " + at(0.5) + " / " + at(0.9);
}

void row(const std::string& label, int frames, int ofFrames, int windows, int ofWindows) {
    std::cout << "  " << align(label, -35) << align(percent(frames, ofFrames), -15)
              << percent(windows, ofWindows) << "\n";
}

/// {seconds:0.#}
std::string seconds(double value) {
    return formatFixed(value, 0, 1);
}

int run(const std::vector<std::string>& args) {
    std::optional<std::string> path;
    std::optional<std::pair<float, float>> at;
    float radius = 0.5f;
    float zMin = -0.2f;
    float zMax = 2.2f;
    float height = 1.2f;
    double windowSeconds = 3;
    int enough = 3;
    bool each = false;

    try {
        for (std::size_t a = 0; a < args.size(); ++a) {
            const auto next = [&]() -> const std::string& {
                if (++a < args.size()) return args[a];
                throw UsageError(args[a - 1] + " needs a value");
            };
            const std::string& arg = args[a];
            if (arg == "--at") {
                const std::string& xy = next();
                const auto comma = xy.find(',');
                if (comma == std::string::npos || xy.find(',', comma + 1) != std::string::npos)
                    throw UsageError("--at takes X,Y, e.g. --at 0.4,3.0");
                at = {number(xy.substr(0, comma)), number(xy.substr(comma + 1))};
            } else if (arg == "--radius") {
                radius = number(next());
            } else if (arg == "--zmin") {
                zMin = number(next());
            } else if (arg == "--zmax") {
                zMax = number(next());
            } else if (arg == "--height") {
                height = number(next());
            } else if (arg == "--window") {
                windowSeconds = number(next());
            } else if (arg == "--min") {
                enough = static_cast<int>(number(next()));
            } else if (arg == "--each") {
                each = true;
            } else {
                if (arg.starts_with("--") || path) throw UsageError("Unknown argument: " + arg);
                path = arg;
            }
        }
    } catch (const UsageError& ex) {
        return usage(ex.what());
    }

    if (!path || !at) return usage("Give a recording and --at X,Y.");
    if (std::filesystem::is_directory(*path))
        path = (std::filesystem::path(*path) / "default_session.swrec").string();
    if (!std::filesystem::is_regular_file(*path)) return usage("No recording at " + *path);
    const auto [spotX, spotY] = *at;

    const auto recording = MMWave::Recording::FrameRecording::load(*path);
    // The same gate and tilt correction as the dashboards, from the .swimu beside the recording.
    auto recordedGate = ImuFrames::loadRecordedGate(*path);
    Imu::Gating::MotionGate* gate = recordedGate ? recordedGate->gate.get() : nullptr;

    const auto frames = recording.frames();
    const auto times = recording.timestamps();
    if (frames.empty()) return usage(*path + " has no frames.");

    // TimeSpan.FromSeconds: the ticks, truncated.
    const TimeSpan window(static_cast<std::int64_t>(windowSeconds * 10'000'000.0));
    std::vector<Window> windows;
    std::vector<int> perFrameAt;
    std::vector<int> perFrameElsewhere;
    std::vector<float> dopplerAt;
    std::vector<float> dopplerElsewhere;
    std::vector<float> snrAt;
    std::vector<float> snrElsewhere;
    int refused = 0;
    int malformed = static_cast<int>(recording.malformedFrames().size());
    int atCap = 0;
    int noCloud = 0;

    for (std::size_t i = 0; i < frames.size(); ++i) {
        const auto w = static_cast<std::size_t>(static_cast<double>((times[i] - times[0]).count()) /
                                                static_cast<double>(window.count()));
        while (windows.size() <= w)
            windows.push_back({times[0] + window * static_cast<std::int64_t>(windows.size())});
        windows[w].frames++;

        EntryPackage package;
        try {
            package = EntryPackage::unpack(frames[i]);
        } catch (const MMWave::Exceptions::FrameEntryMalformedException&) {
            malformed++;
            continue;
        }

        if (package.pointCloud && package.pointCloud->points.count() >= PointCap) atCap++;

        // Frames the gate refuses (device moving or settling) are left out, as the dataset
        // leaves them out.
        std::optional<Imu::Gating::ImuFrameState> imu;
        if (gate) imu = gate->evaluate(times[i]);
        if (imu && !imu->accepted) {
            refused++;
            continue;
        }
        windows[w].kept++;

        int countAt = 0;
        int countElsewhere = 0;
        if (package.pointCloud) {
            // Levelled exactly as the app levels them, then lifted to the height the radar was
            // held at.
            const MMWave::Quaternion leveling =
                imu ? imu->tilt.rotation() : MMWave::Quaternion::identity();
            const PointEntry entry(*package.pointCloud, times[i], leveling);
            for (const auto& point : entry.pointValues()) {
                const Vector3 p = point.cartesian + Vector3{0, 0, height};
                const float dx = p.x - spotX;
                const float dy = p.y - spotY;
                const bool inside =
                    dx * dx + dy * dy <= radius * radius && p.z >= zMin && p.z <= zMax;
                if (inside) {
                    countAt++;
                    dopplerAt.push_back(std::abs(point.doppler));
                    snrAt.push_back(point.snr);
                } else {
                    countElsewhere++;
                    dopplerElsewhere.push_back(std::abs(point.doppler));
                    snrElsewhere.push_back(point.snr);
                }
            }
        } else {
            noCloud++;
        }

        perFrameAt.push_back(countAt);
        perFrameElsewhere.push_back(countElsewhere);
        windows[w].pointsAt += countAt;
        windows[w].pointsElsewhere += countElsewhere;
    }

    const int kept = static_cast<int>(perFrameAt.size());
    // A window counts only if the device was steady for most of it, as a hold window should be.
    std::vector<const Window*> usable;
    for (const Window& x : windows)
        if (x.kept * 2 >= x.frames && x.kept > 0) usable.push_back(&x);
    const auto usableCount = [&](auto&& predicate) {
        return static_cast<int>(std::ranges::count_if(usable, predicate));
    };
    const auto frameCount = [&](int least) {
        return static_cast<int>(
            std::ranges::count_if(perFrameAt, [&](int c) { return c >= least; }));
    };
    const int total = static_cast<int>(frames.size());
    const int windowCount = static_cast<int>(windows.size());
    const int usableTotal = static_cast<int>(usable.size());

    std::cout << *path << "\n";
    std::cout << "  person at X " << SarwaveTools::signedFixed(spotX, 2) << " m, Y "
              << formatFixed(spotY, 2, 2) << " m (radius " << formatFixed(radius, 2, 2)
              << " m, height " << formatFixed(zMin, 1, 1) << " to " << formatFixed(zMax, 1, 1)
              << " m), radar held at " << formatFixed(height, 2, 2) << " m\n";
    std::cout << "  "
              << formatFixed(std::chrono::duration<double>(recording.duration()).count(), 1, 1)
              << " s, " << total << " frames: kept " << kept << " (" << percent(kept, total)
              << "), refused by the gate " << refused << ", malformed " << malformed
              << ", without a point cloud " << noCloud << "\n";
    if (!gate) {
        std::cout << "  ! no IMU recording (.swimu) beside it: no motion gate or tilt correction, "
                     "so heights are only right if the device was held level\n";
    }
    if (kept == 0) {
        std::cout << "  ! no frame was kept: the device was never steady; record it again"
                  << std::endl;
        return 1;
    }
    std::cout << "  " << usableTotal << " of " << windowCount << " " << seconds(windowSeconds)
              << " s windows usable (steady for at least half of it)\n";
    std::cout << "\n";
    std::cout << "                                     per frame      per "
              << seconds(windowSeconds) << " s window\n";
    row("with a point at the person", frameCount(1), kept,
        usableCount([](const Window* x) { return x->pointsAt >= 1; }), usableTotal);
    row("with " + std::to_string(enough) + " or more points there", frameCount(enough), kept,
        usableCount([&](const Window* x) { return x->pointsAt >= enough; }), usableTotal);

    std::vector<int> windowAt;
    std::vector<int> windowElsewhere;
    for (const Window* x : usable) {
        windowAt.push_back(x->pointsAt);
        windowElsewhere.push_back(x->pointsElsewhere);
    }
    std::cout << "  points at the person (median)      " << align(median(perFrameAt), -15)
              << median(windowAt) << "\n";
    std::cout << "  points elsewhere (median)          " << align(median(perFrameElsewhere), -15)
              << median(windowElsewhere) << "\n";
    std::cout << "  " << align("frames at the " + std::to_string(PointCap) + "-point cap", -35)
              << atCap << (atCap > 0 ? "  ! point counts are cut off in these" : "") << "\n";
    std::cout << "\n";
    std::cout << "                                     at the person          elsewhere\n";
    std::cout << "  speed |Doppler|, m/s, 10/50/90 %   " << align(spread(dopplerAt, 2), -23)
              << spread(dopplerElsewhere, 2) << "\n";
    std::cout << "  SNR (linear), 10/50/90 %           " << align(spread(snrAt, 0), -23)
              << spread(snrElsewhere, 0) << "\n";

    if (each) {
        std::cout << "\n";
        std::cout << "  window    frames kept   at the person   elsewhere\n";
        for (const Window& x : windows) {
            const bool isUsable = std::ranges::find(usable, &x) != usable.end();
            std::cout << "  "
                      << align(formatFixed(
                                   std::chrono::duration<double>(x.start - times[0]).count(), 1, 1),
                               5)
                      << " s   " << align(std::to_string(x.kept), 4) << " / "
                      << align(std::to_string(x.frames), -4) << "   "
                      << align(std::to_string(x.pointsAt), 13) << "   "
                      << align(std::to_string(x.pointsElsewhere), 9)
                      << (isUsable ? "" : "   (not steady)") << "\n";
        }
    }
    std::cout << std::flush;
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
