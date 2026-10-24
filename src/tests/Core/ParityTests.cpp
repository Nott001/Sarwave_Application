// Decodes the fixtures in src/tests/data/parity with the C++ port and requires exactly the text
// the original C# code printed for them (see that folder's README).

#include <fstream>
#include <iterator>
#include <sstream>
#include <string>

#include "MMWave/Exceptions/FrameMalformedException.hpp"
#include "MMWave/Recording/FrameRecording.hpp"
#include "MMWave/Streaming/FrameReader.hpp"
#include "Parity/ParityDump.hpp"
#include "TestFramework.hpp"

namespace {

using namespace Sarwave::Core::MMWave;

const std::string Dir = SARWAVE_TEST_DATA_DIR "/parity/";

std::string readText(const std::string& name) {
    std::ifstream in(Dir + name, std::ios::binary);
    REQUIRE(in.is_open());
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

/// Compares line by line so a failure names the first differing line.
void checkSameText(const std::string& actual, const std::string& expected,
                   const std::string& what) {
    if (actual == expected) return;
    std::istringstream a(actual);
    std::istringstream e(expected);
    std::string la;
    std::string le;
    int line = 1;
    while (true) {
        const bool ga = static_cast<bool>(std::getline(a, la));
        const bool ge = static_cast<bool>(std::getline(e, le));
        if (!ga && !ge) break;
        if (la != le || ga != ge) {
            SarwaveTest::fail(__FILE__, __LINE__,
                              what + ": first difference at line " + std::to_string(line) +
                                  "\n      C++: [" + la + "]\n      C#:  [" + le + "]");
            return;
        }
        ++line;
    }
    SarwaveTest::fail(__FILE__, __LINE__, what + ": texts differ (line endings?)");
}

void checkLog(const std::string& name, bool hideBytes) {
    const auto rec = Recording::FrameRecording::load(Dir + name + ".swrec");
    std::ostringstream out;
    SarwaveParity::dumpLog(out, rec, hideBytes);
    checkSameText(out.str(), readText(name + (hideBytes ? ".log1.txt" : ".log0.txt")),
                  name + " DebugLogger");
}

void checkManager(const std::string& name, int durationMs) {
    const auto rec = Recording::FrameRecording::load(Dir + name + ".swrec");
    std::ostringstream out;
    SarwaveParity::dumpManager(out, rec, durationMs);
    checkSameText(out.str(), readText(name + ".mgr" + std::to_string(durationMs) + ".txt"),
                  name + " CombinedManager " + std::to_string(durationMs) + " ms");
}

void checkImuManager(int offsetMs) {
    const auto rec = Recording::FrameRecording::load(Dir + "session.swrec");
    std::ostringstream out;
    SarwaveParity::dumpImuManager(out, rec, SARWAVE_TEST_DATA_DIR "/imu/session.swimu", 120,
                                  offsetMs,
                                  SARWAVE_TEST_DATA_DIR "/../../../configs/Sarwave_12m.cfg");
    checkSameText(out.str(), readText("session.imu" + std::to_string(offsetMs) + ".txt"),
                  "IMU-gated CombinedManager at " + std::to_string(offsetMs) + " ms");
}

}  // namespace

TEST_CASE("parity: DebugLogger output equals the C# DebugLogger's") {
    for (const char* name : {"session", "reuse", "unknown_tlv"}) {
        checkLog(name, false);
        checkLog(name, true);
    }
}

TEST_CASE("parity: CombinedManager state equals the C# CombinedManager's") {
    checkManager("session", 120);
    checkManager("reuse", 1000);
    checkManager("reuse", 120);
    checkManager("unknown_tlv", 1000);
}

TEST_CASE("parity: the IMU-gated CombinedManager equals the C# one") {
    // From a pitch-up with settling, and from a flick, a knock and walking.
    checkImuManager(5000);
    checkImuManager(15000);
}

TEST_CASE("parity: FrameReader extracts the same frames as the C# FrameReader") {
    const std::string raw = readText("stream.bin");
    Streaming::FrameReader reader;
    std::ostringstream out;
    std::uint32_t seed = 7;
    std::size_t pos = 0;
    int count = 0;
    while (true) {
        Streaming::Frame frame;
        bool extracted = false;
        try {
            extracted = reader.tryExtractFrame(frame);
        } catch (const Exceptions::FrameReaderMalformedException& m) {
            out << "malformed " << Exceptions::toString(m.cause()) << " "
                << (m.frameNumber() ? std::to_string(*m.frameNumber()) : "-") << ": " << m.detail()
                << " skipped=" << reader.skippedFrames() << "\n";
            continue;
        }
        if (extracted) {
            std::uint32_t sum = 0;
            for (std::uint8_t x : frame.bytes()) sum = sum * 31 + x;
            out << "frame " << count++ << " len=" << frame.bytes().size() << " sum=" << sum << "\n";
            continue;
        }
        if (pos >= raw.size()) break;
        seed = seed * 1103515245u + 12345u;
        const std::size_t chunk = std::min<std::size_t>(1 + (seed >> 8) % 700u, raw.size() - pos);
        reader.append({reinterpret_cast<const std::uint8_t*>(raw.data()) + pos, chunk});
        pos += chunk;
    }
    out << "end buffered=" << reader.bufferedCount() << "\n";
    checkSameText(out.str(), readText("stream.reader7.txt"), "FrameReader");
}
