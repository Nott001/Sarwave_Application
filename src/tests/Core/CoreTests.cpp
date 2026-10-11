#include <chrono>
#include <cmath>
#include <limits>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include "FrameBuilder.hpp"
#include "MMWave/Common/DotNetCollections.hpp"
#include "MMWave/Common/DotNetFormat.hpp"
#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Configured/PeopleTracking/Association/CombinedManager.hpp"
#include "MMWave/Prompting/Prompting.hpp"
#include "MMWave/Recording/FramePlayback.hpp"
#include "MMWave/Recording/FrameRecorder.hpp"
#include "MMWave/Recording/FrameRecording.hpp"
#include "MMWave/Streaming/FrameReader.hpp"
#include "MMWave/Tlv/TlvRange.hpp"
#include "TestFramework.hpp"

using namespace Sarwave::Core;
using namespace Sarwave::Core::MMWave;
using namespace Sarwave::Core::MMWave::Configured::PeopleTracking;
using SarwaveTest::FrameBuilder;
using namespace std::string_literals;

// ---- .NET formatting --------------------------------------------------------------------------

TEST_CASE("formatSingle matches float.ToString()") {
    CHECK_EQ(DotNet::formatSingle(0.0f), "0");
    CHECK_EQ(DotNet::formatSingle(-0.0f), "-0");
    CHECK_EQ(DotNet::formatSingle(1.5f), "1.5");
    CHECK_EQ(DotNet::formatSingle(0.1f), "0.1");
    CHECK_EQ(DotNet::formatSingle(1.0f / 3.0f), "0.33333334");
    CHECK_EQ(DotNet::formatSingle(100000.0f), "100000");
    CHECK_EQ(DotNet::formatSingle(1e9f), "1E+09");
    CHECK_EQ(DotNet::formatSingle(123456789.0f), "123456790");
    CHECK_EQ(DotNet::formatSingle(0.0001f), "0.0001");
    CHECK_EQ(DotNet::formatSingle(0.00001f), "1E-05");
    CHECK_EQ(DotNet::formatSingle(-1.25e-7f), "-1.25E-07");
    CHECK_EQ(DotNet::formatSingle(std::numeric_limits<float>::infinity()), "Infinity");
    CHECK_EQ(DotNet::formatSingle(std::numeric_limits<float>::quiet_NaN()), "NaN");
}

TEST_CASE("formatFixed matches .NET custom formats for double and float") {
    CHECK_EQ(DotNet::formatFixed(2.5, 0, 0), "3");  // away from zero
    CHECK_EQ(DotNet::formatFixed(0.125, 2, 2), "0.13");
    CHECK_EQ(DotNet::formatFixed(0.12499999, 2, 2), "0.12");   // double: 15 digits
    CHECK_EQ(DotNet::formatFixed(0.12499999f, 2, 2), "0.13");  // float: 7 digits
    CHECK_EQ(DotNet::formatFixed(2.5, 0, 1), "2.5");
    CHECK_EQ(DotNet::formatFixed(2.0, 0, 2), "2");
    CHECK_EQ(DotNet::formatFixed(-0.001, 2, 2), "-0.00");  // .NET keeps the sign
    CHECK_EQ(DotNet::formatFixed(99.995, 2, 2), "100.00");
}

TEST_CASE("formatMinutesSecondsMicros is TimeSpan mm\\:ss\\.ffffff") {
    CHECK_EQ(DotNet::formatMinutesSecondsMicros(TimeSpan(12345678)), "00:01.234567");
    CHECK_EQ(DotNet::formatMinutesSecondsMicros(TimeSpan(36123456789)),
             "00:12.345678");  // hours dropped
    CHECK_EQ(DotNet::formatMinutesSecondsMicros(TimeSpan(9)), "00:00.000000");
}

TEST_CASE("tryParseInt32 / tryParseSingle follow .NET parsing") {
    std::int32_t i = 0;
    CHECK(DotNet::tryParseInt32(" +7 ", i) && i == 7);
    CHECK(!DotNet::tryParseInt32("1.0", i));
    CHECK(!DotNet::tryParseInt32("2147483648", i));
    CHECK(DotNet::tryParseInt32("-2147483648", i) && i == -2147483648);
    float f = 0;
    CHECK(DotNet::tryParseSingle("2950.00", f) && f == 2950.0f);
    CHECK(DotNet::tryParseSingle("1e3", f) && f == 1000.0f);
    CHECK(DotNet::tryParseSingle("Infinity", f) && std::isinf(f));
    CHECK(!DotNet::tryParseSingle("inf", f));
    CHECK(!DotNet::tryParseSingle("0x10", f));
}

// ---- prompts ----------------------------------------------------------------------------------

TEST_CASE("checkStrArgument reproduces the C# validation messages") {
    CHECK_EQ(Prompting::General::checkStrArgument(" 3"), 3);
    try {
        Prompting::General::checkStrArgument("abc");
        CHECK(false);
    } catch (const ArgumentException& e) {
        CHECK_EQ(std::string(e.what()), "Failed to parse integer from input 'abc'");
    }
    try {
        Prompting::General::checkStrArgument("-2");
        CHECK(false);
    } catch (const ArgumentOutOfRangeException& e) {
        CHECK_EQ(std::string(e.what()),
                 "Specified argument was out of the range of valid values. (Parameter "
                 "'Port number cannot be negative (received: -2)')");
    }
#ifdef __linux__
    CHECK_EQ(Prompting::General::createPath(0, 1).data, "/dev/ttyUSB1");
#endif
}

// ---- collections ------------------------------------------------------------------------------

TEST_CASE("DotNetDictionary enumerates like .NET: freed slots are reused, last freed first") {
    DotNetDictionary<int, int> d;
    for (int k : {10, 11, 12, 13}) d.add(k, k);
    d.remove(11);
    d.remove(13);
    d.add(20, 0);  // takes 13's slot
    d.add(21, 0);  // takes 11's slot
    std::vector<int> order;
    d.forEach([&](int k, int) { order.push_back(k); });
    CHECK((order == std::vector<int>{10, 21, 12, 20}));
    CHECK_THROWS_AS(d.add(10, 1), ArgumentException);
}

// ---- framing ----------------------------------------------------------------------------------

TEST_CASE("FrameReader resynchronises on garbage, false magic words and corrupt headers") {
    const auto good = FrameBuilder().presence(true).bytes(1);
    std::vector<std::uint8_t> stream = {0xAA, 0x02, 0x01, 0x04};  // garbage + partial magic
    // A corrupt header (length 5) must be skipped.
    auto corrupt = FrameBuilder().bytes();
    corrupt[12] = 5;
    corrupt[13] = corrupt[14] = corrupt[15] = 0;
    stream.insert(stream.end(), corrupt.begin(), corrupt.end());
    // A frame cut short by a new magic word is dropped in favour of the new one.
    stream.insert(stream.end(), good.begin(), good.begin() + 20);
    stream.insert(stream.end(), good.begin(), good.end());

    Streaming::FrameReader reader;
    reader.append(stream);
    Streaming::Frame frame;
    REQUIRE(reader.tryExtractFrame(frame));
    CHECK(std::vector<std::uint8_t>(frame.bytes().begin(), frame.bytes().end()) == good);
    CHECK(!reader.tryExtractFrame(frame));
}

TEST_CASE("TlvRange stops at a TLV that runs past the frame") {
    auto bytes = FrameBuilder().presence(true).presence(false).bytes();
    bytes[24] = 3;  // numTlvs claims 3
    const auto frame = Streaming::Frame::copyFrom(bytes);
    int n = 0;
    for (const auto& tlv : MMWave::Tlv::TlvRange(frame)) {
        CHECK_EQ(tlv.type, 1021u);
        ++n;
    }
    CHECK_EQ(n, 2);
}

// ---- association ------------------------------------------------------------------------------

TEST_CASE("CombinedManager keeps object ids stable across wire-id reuse") {
    Association::CombinedManager m(TimeSpans::fromSeconds(1));
    m.update(FrameBuilder().targets({1, 2}).cloud(3).presence(true).frame(), TimeSpan(0));
    CHECK(m.presence());
    REQUIRE(m.allTargetObjects().size() == 2);

    // Wire id 1 disappears; its object is kept (dropped) while points refer to it.
    m.update(FrameBuilder().targets({2}).indexes({1, 2, 253}).cloud(2).frame(), TimeSpan(500'000));
    const auto targets = m.allTargetObjects();
    REQUIRE(targets.size() == 2);
    CHECK(targets[0]->state() == Association::TargetObject::TargetState::Dropped);
    CHECK(targets[1]->state() == Association::TargetObject::TargetState::Active);

    // Wire id 1 comes back as a new target: a new object id.
    m.update(FrameBuilder().targets({1, 2}).heights({1}, 1.8f, 0.1f).frame(), TimeSpan(600'000));
    bool sawNewId = false;
    for (const auto* t : m.allTargetObjects()) {
        if (t->objectId() == 2) {
            sawNewId = true;
            CHECK(t->maxZ == 1.8f);
        }
    }
    CHECK(sawNewId);

    // Once the old clouds age out, the dropped object goes too.
    m.update(FrameBuilder().targets({1, 2}).frame(), TimeSpan(30'000'000));
    for (const auto* t : m.allTargetObjects())
        CHECK(t->state() == Association::TargetObject::TargetState::Active);
}

TEST_CASE("EntryPackage rejects TLV types the demo never sends") {
    const auto frame = FrameBuilder().tlv(999, {1, 2, 3, 4}).frame();
    Association::CombinedManager m(TimeSpans::fromSeconds(1));
    try {
        m.update(frame, TimeSpan(0));
        CHECK(false);
    } catch (const ArgumentOutOfRangeException& e) {
        CHECK_EQ(std::string(e.what()), "Unknown TlvType 999. (Parameter 'frame')");
    }
}

// ---- recording --------------------------------------------------------------------------------

TEST_CASE("FrameRecorder and FrameRecording round-trip, and truncation is detected") {
    std::stringstream file;
    {
        Recording::FrameRecorder recorder(file, true);
        recorder.record(FrameBuilder().presence(true).frame(), TimeSpan(5));
        recorder.record(FrameBuilder().targets({3}).frame(), TimeSpan(550'000));
        CHECK_EQ(recorder.frameCount(), 2);
        recorder.dispose();
        CHECK_THROWS_AS(recorder.record(FrameBuilder().frame(), TimeSpan(0)),
                        ObjectDisposedException);
    }
    const std::string bytes = file.str();
    CHECK_EQ(bytes.substr(0, 8), "SWMMWREC");

    std::istringstream in(bytes);
    auto rec = Recording::FrameRecording::load(in);
    CHECK_EQ(rec.count(), 2);
    CHECK(!rec.isTruncated());
    CHECK_EQ(rec.duration().count(), 549'995);

    std::istringstream cut(bytes.substr(0, bytes.size() - 3));
    auto truncated = Recording::FrameRecording::load(cut);
    CHECK_EQ(truncated.count(), 1);
    CHECK(truncated.isTruncated());

    std::istringstream bad("SWMMWREC\x02\0\0\0"s);
    CHECK_THROWS_AS(Recording::FrameRecording::load(bad), InvalidDataException);
}

TEST_CASE("FramePlayback paces frames by timestamp and loops forward in time") {
    std::stringstream file;
    {
        Recording::FrameRecorder recorder(file, true);
        for (int i = 0; i < 3; ++i)
            recorder.record(FrameBuilder().presence(true).frame(), TimeSpan(i * 100'000));
    }
    std::istringstream in(file.str());
    const auto rec = Recording::FrameRecording::load(in);

    std::mutex gate;
    std::vector<std::int64_t> seen;
    Recording::FramePlayback playback(rec);
    playback.loop = true;
    playback.speed = 2;
    const auto start = std::chrono::steady_clock::now();
    playback.begin([&](const Streaming::Frame&, TimeSpan ts) {
        std::lock_guard lock(gate);
        seen.push_back(ts.count());
    });
    while (true) {
        {
            std::lock_guard lock(gate);
            if (seen.size() >= 7) break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    playback.stop();
    const auto elapsed = std::chrono::steady_clock::now() - start;
    CHECK(!playback.isPlaying());

    // Pass length = duration (200 ms) + one frame gap (100 ms); speed 2 halves the waits.
    const std::vector<std::int64_t> expected{0,       100'000, 200'000, 300'000,
                                             400'000, 500'000, 600'000};
    CHECK((std::vector<std::int64_t>(seen.begin(), seen.begin() + 7) == expected));
    CHECK(elapsed >=
          std::chrono::milliseconds(29));  // the 7th frame is due 60 ms / 2 after the first
}
