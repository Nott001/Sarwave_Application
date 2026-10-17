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
#include "MMWave/Exceptions/FrameMalformedException.hpp"
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

namespace {

using Exceptions::FrameEntryMalformedException;
using Exceptions::FrameReaderMalformedException;

/// Pulls everything out of the reader: "frame N" for each frame, "<Cause> N" for each skipped
/// packet (N = its frame number, or "-" when unknown).
std::vector<std::string> drain(Streaming::FrameReader& reader) {
    std::vector<std::string> events;
    while (true) {
        Streaming::Frame frame;
        try {
            if (!reader.tryExtractFrame(frame)) return events;
            events.push_back("frame " + std::to_string(frame.header().frameNumber));
        } catch (const FrameReaderMalformedException& e) {
            events.push_back(Exceptions::toString(e.cause()) + " " +
                             (e.frameNumber() ? std::to_string(*e.frameNumber()) : "-"));
        }
    }
}

void append(std::vector<std::uint8_t>& to, const std::vector<std::uint8_t>& bytes) {
    to.insert(to.end(), bytes.begin(), bytes.end());
}

}  // namespace

TEST_CASE("FrameReader resynchronises on garbage, false magic words and corrupt headers") {
    const auto good = FrameBuilder().presence(true).bytes(1);
    std::vector<std::uint8_t> stream = {0xAA, 0x02, 0x01, 0x04};  // garbage + partial magic
    // A corrupt header (length 5) is skipped and reported.
    auto corrupt = FrameBuilder().bytes();
    corrupt[12] = 5;
    corrupt[13] = corrupt[14] = corrupt[15] = 0;
    append(stream, corrupt);
    // A frame cut short by a new magic word is dropped in favour of the new one.
    stream.insert(stream.end(), good.begin(), good.begin() + 20);
    append(stream, good);

    Streaming::FrameReader reader;
    reader.append(stream);
    CHECK((drain(reader) == std::vector<std::string>{"Header -", "Interrupted -", "frame 1"}));
    CHECK_EQ(reader.skippedFrames(), 2);

    // The frame itself is intact.
    reader.append(good);
    Streaming::Frame frame;
    REQUIRE(reader.tryExtractFrame(frame));
    CHECK(std::vector<std::uint8_t>(frame.bytes().begin(), frame.bytes().end()) == good);

    reader.reset();
    CHECK_EQ(reader.skippedFrames(), 0);
}

TEST_CASE("FrameReader rejects a length that is not a multiple of 32") {
    Streaming::FrameReader reader;
    reader.append(FrameBuilder().presence(true).unpadded().bytes(4));  // 60 bytes
    reader.append(FrameBuilder().presence(true).bytes(5));
    CHECK((drain(reader) == std::vector<std::string>{"Header -", "frame 5"}));
}

TEST_CASE("FrameReader skips a packet whose TLVs no longer chain, keeping the next one") {
    // Recording D on the Pi: 7 bytes lost from the middle of a target list. The packet is made
    // up to its announced length with the start of the next one, which must still be found.
    auto damaged = FrameBuilder().targets({1}).heights({1}, 1.7f, 0.2f).presence(true).bytes(341);
    damaged.erase(damaged.begin() + 90, damaged.begin() + 97);
    std::vector<std::uint8_t> stream = damaged;
    append(stream, FrameBuilder().presence(true).bytes(342));
    append(stream, FrameBuilder().presence(false).bytes(343));

    Streaming::FrameReader reader;
    // One byte at a time, as a slow serial port might deliver it.
    std::vector<std::string> events;
    for (std::uint8_t b : stream) {
        reader.append(std::span(&b, 1));
        for (auto& e : drain(reader)) events.push_back(e);
    }
    CHECK((events == std::vector<std::string>{"TlvChain 341", "frame 342", "frame 343"}));
}

TEST_CASE("FrameReader leaves a borrowed partial magic word for the next packet") {
    // Bytes lost in the padding: the TLVs still chain, and the packet ends with the first bytes
    // of the next magic word, which must not be swallowed with it.
    auto first = FrameBuilder().presence(true).bytes(10);
    first.resize(first.size() - 3);
    std::vector<std::uint8_t> stream = first;
    append(stream, FrameBuilder().presence(true).bytes(11));

    Streaming::FrameReader reader;
    reader.append(stream);
    CHECK((drain(reader) == std::vector<std::string>{"frame 10", "frame 11"}));
    CHECK_EQ(reader.skippedFrames(), 0);
}

TEST_CASE("TlvRange stops at a TLV that runs past the frame") {
    auto bytes = FrameBuilder().presence(true).presence(false).bytes();
    bytes[32] = 3;  // numTlvs claims 3
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
    m.update(
        FrameBuilder().targets({1, 2}).heights({1, 2}, 1.7f, 0.2f).cloud(3).presence(true).frame(0),
        TimeSpan(0));
    CHECK(m.presence());
    REQUIRE(m.allTargetObjects().size() == 2);

    // Wire id 1 disappears; its object is kept (dropped) while points refer to it.
    m.update(
        FrameBuilder().targets({2}).heights({2}, 1.7f, 0.2f).indexes({1, 2, 253}).cloud(2).frame(1),
        TimeSpan(500'000));
    const auto targets = m.allTargetObjects();
    REQUIRE(targets.size() == 2);
    CHECK(targets[0]->state() == Association::TargetObject::TargetState::Dropped);
    CHECK(targets[1]->state() == Association::TargetObject::TargetState::Active);

    // Wire id 1 comes back as a new target: a new object id.
    m.update(FrameBuilder().targets({1, 2}).heights({1, 2}, 1.8f, 0.1f).frame(2),
             TimeSpan(600'000));
    bool sawNewId = false;
    for (const auto* t : m.allTargetObjects()) {
        if (t->objectId() == 2) {
            sawNewId = true;
            CHECK(t->maxZ == 1.8f);
        }
    }
    CHECK(sawNewId);

    // Once the old clouds age out, the dropped object goes too.
    m.update(FrameBuilder().targets({1, 2}).heights({1, 2}, 1.8f, 0.1f).frame(3),
             TimeSpan(30'000'000));
    for (const auto* t : m.allTargetObjects())
        CHECK(t->state() == Association::TargetObject::TargetState::Active);
}

namespace {

/// The cause and message unpack() gives for a frame, or "ok".
std::string unpackError(const Streaming::Frame& frame) {
    try {
        (void)Association::EntryPackage::unpack(frame);
        return "ok";
    } catch (const FrameEntryMalformedException& e) {
        return Exceptions::toString(e.cause()) + ": " + e.what();
    }
}

std::vector<std::uint8_t> le32(std::uint32_t v) {
    return {static_cast<std::uint8_t>(v), static_cast<std::uint8_t>(v >> 8),
            static_cast<std::uint8_t>(v >> 16), static_cast<std::uint8_t>(v >> 24)};
}

}  // namespace

TEST_CASE("EntryPackage rejects frames the device cannot have sent") {
    CHECK_EQ(unpackError(FrameBuilder().tlv(999, {1, 2, 3, 4}).frame(7)),
             "UnknownTlv: Frame 7: TLV type 999 is not part of the people tracking output.");
    CHECK_EQ(unpackError(FrameBuilder().presence(true).presence(true).frame(1)),
             "DuplicateTlv: Frame 1: TlvPresenceIndication appears more than once.");
    CHECK_EQ(unpackError(FrameBuilder().tlv(1021, le32(2)).frame(1)),
             "Presence: Frame 1: Presence is 2.");
    CHECK_EQ(unpackError(FrameBuilder().tlv(1021, {1, 0}).frame(1)),
             "PayloadLength: Frame 1: TlvPresenceIndication is 2 bytes instead of 4.");
    CHECK_EQ(unpackError(FrameBuilder().cloud(3).detectedObjects(4).frame(2)),
             "PointCount: Frame 2: The header gives 4 points but the point cloud holds 3.");
    CHECK_EQ(unpackError(FrameBuilder().targets({1}).frame(3)),
             "HeightMismatch: Frame 3: 1 targets are listed but 0 heights.");
    CHECK_EQ(
        unpackError(FrameBuilder().targets({1, 2}).heights({1, 3}, 1, 0).frame(3)),
        "HeightMismatch: Frame 3: Target 1 is track 2 in the list but track 3 in the heights.");
    CHECK_EQ(unpackError(FrameBuilder().targets({250}).heights({250}, 1, 0).frame(4)),
             "TrackId: Frame 4: TlvTargetList record 0 has track ID 250.");
    CHECK_EQ(unpackError(FrameBuilder().indexes({1, 251, 253}).frame(5)),
             "PointIndex: Frame 5: Target index 1 is 251.");
    CHECK_EQ(unpackError(FrameBuilder().tlv(1012, std::vector<std::uint8_t>(13)).frame(6)),
             "PayloadLength: Frame 6: TlvTargetHeight is 13 bytes, which is not 0 plus a multiple "
             "of 12.");

    // A TLV cut off by the end of the packet: fewer fit than the header counts.
    auto bytes = FrameBuilder().presence(true).bytes(8);
    bytes[32] = 2;  // numTlvs
    CHECK_EQ(unpackError(Streaming::Frame::copyFrom(bytes)),
             "Truncated: Frame 8: The header counts 2 TLVs but only 1 fit in the packet.");

    CHECK_EQ(unpackError(FrameBuilder()
                             .targets({1})
                             .heights({1}, 1, 0)
                             .indexes({1, 255})
                             .cloud(2)
                             .presence(true)
                             .frame(9)),
             "ok");
}

TEST_CASE("CombinedManager: a missed frame or a skipped one never gets the next indexes") {
    Association::CombinedManager m(TimeSpans::fromSeconds(1));
    m.update(FrameBuilder().targets({1}).heights({1}, 1.7f, 0.2f).cloud(2).frame(10), TimeSpan(0));
    // Frame 11 never arrived: frame 12's indexes describe frame 11's cloud, not frame 10's.
    m.update(
        FrameBuilder().targets({1}).heights({1}, 1.7f, 0.2f).indexes({1, 1, 1}).cloud(1).frame(12),
        TimeSpan(1));
    REQUIRE(m.pointClouds().size() == 2);
    CHECK(!m.pointClouds()[0]->isAssociated());

    // Frame 13 follows on and is associated with frame 12's cloud.
    m.update(FrameBuilder().targets({1}).heights({1}, 1.7f, 0.2f).indexes({1}).frame(13),
             TimeSpan(2));
    CHECK(m.pointClouds()[1]->isAssociated());

    // A frame that arrived but was unusable: the cloud before it gets no indexes either.
    m.update(FrameBuilder().targets({1}).heights({1}, 1.7f, 0.2f).cloud(2).frame(14), TimeSpan(3));
    m.skipFrame(TimeSpan(4));
    m.update(FrameBuilder().targets({1}).heights({1}, 1.7f, 0.2f).indexes({1, 1}).frame(16),
             TimeSpan(5));
    CHECK(!m.pointClouds().back()->isAssociated());
}

TEST_CASE("CombinedManager levels point positions but leaves radial values") {
    const auto frame = FrameBuilder().cloud(1).frame();
    Association::CombinedManager plain(TimeSpans::fromSeconds(1));
    Association::CombinedManager levelled(TimeSpans::fromSeconds(1));
    plain.update(frame, TimeSpan(0));
    // 90° about X: forward (+Y) becomes up (+Z).
    const float h = std::sqrt(0.5f);
    levelled.update(frame, TimeSpan(0), Quaternion{h, 0, 0, h});

    const auto& p = plain.pointClouds().front()->pointValues()[0];
    const auto& q = levelled.pointClouds().front()->pointValues()[0];
    CHECK(std::abs(q.cartesian.x - p.cartesian.x) < 1e-5f);
    CHECK(std::abs(q.cartesian.z - p.cartesian.y) < 1e-5f);
    CHECK(std::abs(q.cartesian.y + p.cartesian.z) < 1e-5f);
    CHECK_EQ(q.radial.range, p.radial.range);
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

    std::istringstream bad("SWMMWREC\x03\0\0\0"s);
    CHECK_THROWS_AS(Recording::FrameRecording::load(bad), InvalidDataException);

    // Version 1 files (frame records only) are still read.
    std::string v1 = bytes;
    v1[8] = 1;
    std::istringstream old(v1);
    CHECK_EQ(Recording::FrameRecording::load(old).count(), 2);
}

TEST_CASE("Malformed-frame notes round-trip through a recording") {
    std::stringstream file;
    {
        Recording::FrameRecorder recorder(file, true);
        recorder.record(FrameBuilder().presence(true).frame(1), TimeSpan(10));
        recorder.recordMalformed(FrameReaderMalformedException(
                                     FrameReaderMalformedException::Reason::TlvChain,
                                     "Its 3 TLVs do not chain to the end of its 224 bytes.", 2u),
                                 TimeSpan(20));
        recorder.recordMalformed(
            FrameReaderMalformedException(FrameReaderMalformedException::Reason::Header,
                                          "The header announces a packet of 5 bytes."),
            TimeSpan(25));
        recorder.record(FrameBuilder().presence(true).frame(3), TimeSpan(30));
        recorder.recordMalformed(
            FrameEntryMalformedException(FrameEntryMalformedException::Reason::Presence,
                                         "Presence is 7.", 3u),
            TimeSpan(30));
        CHECK_EQ(recorder.frameCount(), 2);
        CHECK_EQ(recorder.malformedCount(), 3);
    }

    std::istringstream in(file.str());
    const auto rec = Recording::FrameRecording::load(in);
    CHECK_EQ(rec.count(), 2);
    REQUIRE(rec.malformedFrames().size() == 3);

    const auto& a = rec.malformedFrames()[0];
    CHECK_EQ(a.frameIndex, 1);
    CHECK_EQ(a.timestamp.count(), 20);
    const auto* reader = dynamic_cast<const FrameReaderMalformedException*>(a.exception.get());
    REQUIRE(reader != nullptr);
    CHECK(reader->cause() == FrameReaderMalformedException::Reason::TlvChain);
    CHECK_EQ(std::string(reader->what()),
             "Frame 2: Its 3 TLVs do not chain to the end of its 224 bytes.");

    CHECK(!rec.malformedFrames()[1].exception->frameNumber());
    const auto& c = rec.malformedFrames()[2];
    CHECK_EQ(c.frameIndex, 2);
    CHECK(dynamic_cast<const FrameEntryMalformedException*>(c.exception.get()) != nullptr);
}

TEST_CASE("FramePlayback replays the reader's notes in order and maps looped timestamps back") {
    std::stringstream file;
    {
        Recording::FrameRecorder recorder(file, true);
        recorder.record(FrameBuilder().presence(true).frame(0), TimeSpan(0));
        recorder.recordMalformed(
            FrameReaderMalformedException(FrameReaderMalformedException::Reason::Interrupted,
                                          "The next packet began after 9 bytes.", 1u),
            TimeSpan(50'000));
        // A frame that was rejected live: in the recording, and rejected again on replay.
        recorder.record(FrameBuilder().tlv(1021, le32(9)).frame(2), TimeSpan(100'000));
        recorder.recordMalformed(
            FrameEntryMalformedException(FrameEntryMalformedException::Reason::Presence,
                                         "Presence is 9.", 2u),
            TimeSpan(100'000));
        recorder.record(FrameBuilder().presence(true).frame(3), TimeSpan(150'000));
    }
    std::istringstream in(file.str());
    const auto rec = Recording::FrameRecording::load(in);

    std::mutex gate;
    std::vector<std::string> events;
    Recording::FramePlayback playback(rec);
    playback.speed = 10;
    playback.begin(
        [&](const Streaming::Frame& frame, TimeSpan ts) {
            (void)Association::EntryPackage::unpack(frame);  // throws for frame 2
            std::lock_guard lock(gate);
            events.push_back("frame " + std::to_string(frame.header().frameNumber) + " @" +
                             std::to_string(ts.count()));
        },
        [&](const Exceptions::FrameMalformedException& ex, TimeSpan ts) {
            std::lock_guard lock(gate);
            events.push_back("malformed " + std::to_string(ex.frameNumber().value_or(99)) + " @" +
                             std::to_string(ts.count()));
        });
    playback.waitForCompletion();
    CHECK(!playback.error());
    CHECK((events == std::vector<std::string>{"frame 0 @0", "malformed 1 @50000",
                                              "malformed 2 @100000", "frame 3 @150000"}));

    // Pass length: the 150 ms duration plus one average gap (150 ms / 2).
    CHECK_EQ(playback.recordedTimestamp(TimeSpan(120'000)).count(), 120'000);
    CHECK_EQ(playback.recordedTimestamp(TimeSpan(250'000)).count(), 25'000);
    CHECK_EQ(playback.recordedTimestamp(TimeSpan(460'000)).count(), 10'000);
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
