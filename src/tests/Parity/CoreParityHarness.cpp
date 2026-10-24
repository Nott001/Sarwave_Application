// Prints what the core library makes of a recording / byte stream in a fixed text layout. The
// C# twin of this program (Parity/Harness.cs, run against the original Sarwave.Core.MMWave)
// prints the same layout; diffing the two outputs proves the port decodes identically.
//
//   CoreParityHarness log <file.swrec> <hideBytes 0|1>     DebugLogger output for every frame
//   CoreParityHarness manager <file.swrec> <durationMs>    CombinedManager state after every frame
//   CoreParityHarness imumanager <file.swrec> <file.swimu> <durationMs> <offsetMs> <file.cfg>
//                                                          the same, gated and moved by the IMU
//   CoreParityHarness reader <raw.bin> <seed>              FrameReader over a chunked byte stream
//   CoreParityHarness load <file.swrec>                    FrameRecording.Load summary / error
//   CoreParityHarness rewrite <in.swrec> <out.swrec>       FrameRecorder round trip
//   CoreParityHarness playback <file.swrec> <speed> <n>    first n looped playback timestamps
//   CoreParityHarness prompt                               port-number parsing
//   CoreParityHarness timespan                             mm:ss.ffffff formatting

#include <algorithm>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <iterator>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "Integration/PeopleTracking/Association/CombinedManager.hpp"
#include "MMWave/Common/DotNetFormat.hpp"
#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Configured/PeopleTracking/DebugLogger.hpp"
#include "MMWave/Exceptions/FrameMalformedException.hpp"
#include "MMWave/Prompting/Prompting.hpp"
#include "MMWave/Recording/FramePlayback.hpp"
#include "MMWave/Recording/FrameRecorder.hpp"
#include "MMWave/Recording/FrameRecording.hpp"
#include "MMWave/Streaming/FrameReader.hpp"

using namespace Sarwave::Core;
using namespace Sarwave::Core::MMWave;
using namespace Sarwave::Core::MMWave::Configured::PeopleTracking;
namespace Association = Sarwave::Integration::PeopleTracking::Association;
using Sarwave::Integration::PeopleTracking::EntryPackage;

#include "ParityDump.hpp"

namespace {

using namespace SarwaveParity;

int cmdReader(const std::string& path, std::uint32_t seed) {
    std::ifstream in(path, std::ios::binary);
    const std::vector<std::uint8_t> raw((std::istreambuf_iterator<char>(in)),
                                        std::istreambuf_iterator<char>());

    Streaming::FrameReader reader;
    std::size_t pos = 0;
    int count = 0;
    while (true) {
        Streaming::Frame frame;
        bool extracted = false;
        try {
            extracted = reader.tryExtractFrame(frame);
        } catch (const Exceptions::FrameReaderMalformedException& m) {
            std::cout << "malformed " << Exceptions::toString(m.cause()) << " "
                      << (m.frameNumber() ? std::to_string(*m.frameNumber()) : "-") << ": "
                      << m.detail() << " skipped=" << reader.skippedFrames() << "\n";
            continue;
        }
        if (extracted) {
            std::uint32_t sum = 0;
            for (std::uint8_t x : frame.bytes()) sum = sum * 31 + x;
            std::cout << "frame " << count++ << " len=" << frame.bytes().size() << " sum=" << sum
                      << "\n";
            continue;
        }
        if (pos >= raw.size()) break;
        seed = seed * 1103515245u + 12345u;
        std::size_t chunk = 1 + (seed >> 8) % 700u;
        chunk = std::min(chunk, raw.size() - pos);
        reader.append(std::span(raw).subspan(pos, chunk));
        pos += chunk;
    }
    std::cout << "end buffered=" << reader.bufferedCount() << "\n";
    return 0;
}

int cmdLoad(const std::string& path) {
    try {
        auto rec = Recording::FrameRecording::load(path);
        std::cout << "count=" << rec.count()
                  << " truncated=" << (rec.isTruncated() ? "True" : "False")
                  << " duration=" << rec.duration().count() << "\n";
    } catch (const std::exception& e) {
        std::cout << Ex(e) << "\n";
    }
    return 0;
}

int cmdRewrite(const std::string& in, const std::string& out) {
    auto rec = Recording::FrameRecording::load(in);
    Recording::FrameRecorder w{std::filesystem::path(out)};
    for (int i = 0; i < rec.count(); i++) {
        w.record(rec.frames()[static_cast<std::size_t>(i)],
                 rec.timestamps()[static_cast<std::size_t>(i)]);
    }
    std::cout << "frames=" << w.frameCount() << "\n";
    return 0;
}

int cmdPlayback(const std::string& path, double speed, int n) {
    auto rec = Recording::FrameRecording::load(path);
    std::vector<std::string> seen;
    std::mutex gate;
    std::condition_variable done;
    Recording::FramePlayback pb(rec);
    pb.speed = speed;
    pb.loop = true;
    pb.begin([&](const Streaming::Frame& frame, TimeSpan ts) {
        std::lock_guard lock(gate);
        if (static_cast<int>(seen.size()) < n)
            seen.push_back(std::to_string(ts.count()) + " " + std::to_string(frame.bytes().size()));
        if (static_cast<int>(seen.size()) == n) done.notify_all();
    });
    {
        std::unique_lock lock(gate);
        done.wait(lock, [&] { return static_cast<int>(seen.size()) >= n; });
    }
    pb.stop();
    for (const auto& s : seen) std::cout << s << "\n";
    return 0;
}

int cmdPrompt() {
    for (const std::string input : {"0", "5", " 7 ", "+3", "-0", "-1", "abc", "", "2147483647",
                                    "2147483648", "1.0", "0x10", "\xD9\xA3"}) {
        try {
            const int port = Prompting::General::checkStrArgument(input);
            std::cout << "'" << input << "' -> " << port << "\n";
        } catch (const std::exception& e) {
            std::cout << "'" << input << "' -> " << Ex(e) << "\n";
        }
    }
    const auto paths = Prompting::General::createPath(0, 1);
    std::cout << paths.cli << " " << paths.data << "\n";
    return 0;
}

int cmdFixed() {
    std::vector<double> vals{0,     -0.0,   0.5,      1.5,    2.5,        -0.5,      0.125, 0.135,
                             1.005, 2.675,  0.001,    -0.001, -0.004,     -0.005,    0.045, 12,
                             12.5,  99.995, 999.9999, 1e-7,   123456.785, 1.25,      -1.25, 0.05,
                             0.95,  0.995,  3.14159,  2.5e10, 1234567.89, 0.12499999};
    std::uint32_t seed = 9;
    for (int i = 0; i < 3000; i++) {
        seed = seed * 1103515245u + 12345u;
        double r = static_cast<double>((seed >> 8) % 2000001) / 1000.0 - 1000.0;
        if (i % 3 == 0) r /= 977.0;
        vals.push_back(r);
    }
    const std::pair<int, int> fmts[] = {{2, 2}, {0, 2}, {0, 1}, {0, 0}, {3, 3}, {1, 1}};
    for (double v : vals) {
        std::string line;
        for (auto [mn, mx] : fmts) line += DotNet::formatFixed(v, mn, mx) + " ";
        for (auto [mn, mx] : fmts) line += DotNet::formatFixed(static_cast<float>(v), mn, mx) + " ";
        line.pop_back();
        std::cout << line << "\n";
    }
    return 0;
}

int cmdTimeSpan() {
    for (long long t : {0LL, 1LL, 9LL, 10LL, 12345678LL, 599999999LL, 600000000LL, 36000000000LL,
                        36123456789LL, 863999999999LL}) {
        std::cout << t << " " << DotNet::formatMinutesSecondsMicros(TimeSpan(t)) << "\n";
    }
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    const std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty()) return 2;
    const std::string& cmd = args[0];
    if (cmd == "log") {
        dumpLog(std::cout, Recording::FrameRecording::load(args[1]), args[2] == "1");
        return 0;
    }
    if (cmd == "manager") {
        dumpManager(std::cout, Recording::FrameRecording::load(args[1]), std::stod(args[2]));
        return 0;
    }
    if (cmd == "imumanager") {
        dumpImuManager(std::cout, Recording::FrameRecording::load(args[1]), args[2],
                       std::stod(args[3]), std::stod(args[4]), args[5]);
        return 0;
    }
    if (cmd == "reader") return cmdReader(args[1], static_cast<std::uint32_t>(std::stoul(args[2])));
    if (cmd == "load") return cmdLoad(args[1]);
    if (cmd == "rewrite") return cmdRewrite(args[1], args[2]);
    if (cmd == "playback") return cmdPlayback(args[1], std::stod(args[2]), std::stoi(args[3]));
    if (cmd == "prompt") return cmdPrompt();
    if (cmd == "timespan") return cmdTimeSpan();
    if (cmd == "fixed") return cmdFixed();
    return 2;
}
