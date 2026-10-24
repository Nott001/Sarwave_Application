// The LAN stream between the live app and the wireless app: its message format, and a sender
// and a receiver talking over the loopback interface.

#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "Integration/PeopleTracking/Lan/LanFormat.hpp"
#include "Integration/PeopleTracking/Lan/LanReceiver.hpp"
#include "Integration/PeopleTracking/Lan/LanSender.hpp"
#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Exceptions/FrameMalformedException.hpp"
#include "MMWave/Streaming/Frame.hpp"
#include "TestFramework.hpp"

namespace {

using namespace Sarwave::Integration::PeopleTracking::Lan;
using Sarwave::Core::InvalidDataException;
using Sarwave::Core::Imu::ImuSample;
using Sarwave::Core::MMWave::TimeSpan;
using Sarwave::Core::MMWave::Exceptions::FrameEntryMalformedException;
using Sarwave::Core::MMWave::Exceptions::FrameMalformedException;
using Sarwave::Core::MMWave::Streaming::Frame;

std::span<const std::uint8_t> payloadOf(const std::vector<std::uint8_t>& message) {
    return std::span(message).subspan(LanFormat::HeaderSize);
}

/// What a receiver was handed, in order, as text.
struct Received {
    std::mutex gate;
    std::condition_variable changed;
    std::vector<std::string> events;
    bool connected = false;

    void add(std::string event) {
        {
            std::lock_guard lock(gate);
            events.push_back(std::move(event));
        }
        changed.notify_all();
    }

    /// Waits up to 5 s for `count` events.
    std::vector<std::string> waitFor(std::size_t count) {
        std::unique_lock lock(gate);
        changed.wait_for(lock, std::chrono::seconds(5), [&] { return events.size() >= count; });
        return events;
    }
};

void startRecording(LanReceiver& receiver, Received& received) {
    receiver.start([&](const std::string& cfg) { received.add("hello " + cfg); },
                   [&](const Frame& frame, TimeSpan timestamp) {
                       if (frame.bytes().size() == 1 && frame.bytes()[0] == 0xEE)
                           throw FrameEntryMalformedException(
                               FrameEntryMalformedException::Reason::Presence, "bad presence", 9);
                       received.add("frame " + std::to_string(timestamp.count()) +
                                    " n=" + std::to_string(frame.bytes().size()));
                   },
                   [&](const FrameMalformedException& ex, TimeSpan timestamp) {
                       received.add("malformed " + std::to_string(timestamp.count()) + " " +
                                    ex.detail() + " #" +
                                    std::to_string(ex.frameNumber().value_or(0)));
                   },
                   [&](std::span<const ImuSample> samples) {
                       received.add("imu " + std::to_string(samples.size()) +
                                    " t=" + std::to_string(samples.back().timestamp.count()));
                   });
}

bool waitUntil(const std::function<bool()>& condition) {
    for (int i = 0; i < 500 && !condition(); ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return condition();
}

}  // namespace

TEST_CASE("LAN messages carry hello, frames, malformed frames and IMU samples whole") {
    CHECK(LanFormat::decodeHello(payloadOf(LanFormat::encodeHello("sensorStop\n"))) ==
          "sensorStop\n");

    const std::vector<std::uint8_t> bytes{1, 2, 3, 4};
    const auto frame = LanFormat::encodeFrame(bytes, TimeSpan(1234567));
    std::uint8_t type = 0;
    std::int32_t length = 0;
    LanFormat::readHeader(std::span(frame).first<LanFormat::HeaderSize>(), type, length);
    CHECK(type == LanFormat::Frame);
    CHECK(length == 12);
    CHECK(LanFormat::readTimestamp(payloadOf(frame)) == TimeSpan(1234567));

    const std::vector<ImuSample> samples{
        {TimeSpan(10), {0.1f, -0.2f, 1.0f}, {1.5f, -2.5f, 3.25f}, 31.5f},
        {TimeSpan(-20), {-0.0f, 2e-9f, -1.0f}, {0, 0, 1e6f}, -40.0f}};
    std::vector<ImuSample> decoded;
    CHECK(LanFormat::decodeImu(payloadOf(LanFormat::encodeImu(samples)), decoded) == 2);
    CHECK(decoded == samples);

    // A bad type or length, another program, a short or miscounted payload.
    const std::uint8_t badType[] = {9, 0, 0, 0, 0};
    CHECK_THROWS_AS(LanFormat::readHeader(badType, type, length), InvalidDataException);
    const std::uint8_t tooLong[] = {2, 0, 0, 0x11, 0};  // past MaxPayload
    CHECK_THROWS_AS(LanFormat::readHeader(tooLong, type, length), InvalidDataException);
    const std::uint8_t http[] = {'H', 'T', 'T', 'P', 1, 0};
    CHECK_THROWS_AS(LanFormat::decodeHello(http), InvalidDataException);
    const std::uint8_t version2[] = {'S', 'W', 'L', 'N', 2, 0};
    CHECK_THROWS_AS(LanFormat::decodeHello(version2), InvalidDataException);
    CHECK_THROWS_AS(LanFormat::readTimestamp(std::span(bytes)), InvalidDataException);
    auto imu = LanFormat::encodeImu(samples);
    imu.pop_back();
    CHECK_THROWS_AS(LanFormat::decodeImu(payloadOf(imu), decoded), InvalidDataException);
}

TEST_CASE("A LAN receiver gets the sender's session, and a newer receiver takes its place") {
    LanSender sender(boost::asio::ip::make_address("127.0.0.1"), 0, "cfg text");
    sender.start();

    // Nothing is queued without a receiver.
    const auto frame = Frame::copyFrom(std::vector<std::uint8_t>(40, 7));
    sender.sendFrame(frame, TimeSpan(100));
    CHECK(sender.dropped() == 0);

    Received first;
    auto receiver = std::make_unique<LanReceiver>("127.0.0.1", sender.port());
    startRecording(*receiver, first);
    REQUIRE(waitUntil([&] { return sender.isConnected(); }));

    sender.sendFrame(frame, TimeSpan(200));
    sender.sendMalformed(
        FrameEntryMalformedException(FrameEntryMalformedException::Reason::TrackId, "track 300", 5),
        TimeSpan(300));
    const std::vector<ImuSample> samples(3, ImuSample{TimeSpan(400), {}, {}, 25.0f});
    sender.sendImu(samples);
    // A frame the receiving side finds malformed goes to its malformed callback.
    sender.sendFrame(Frame::copyFrom(std::vector<std::uint8_t>{0xEE}), TimeSpan(500));

    auto events = first.waitFor(5);
    REQUIRE(events.size() == 5u);
    CHECK(events[0] == "hello cfg text");
    CHECK(events[1] == "frame 200 n=40");
    CHECK(events[2] == "malformed 300 track 300 #5");
    CHECK(events[3] == "imu 3 t=400");
    CHECK(events[4] == "malformed 500 bad presence #9");

    // A second receiver replaces the first and is greeted on its own.
    Received second;
    LanReceiver replacement("127.0.0.1", sender.port());
    startRecording(replacement, second);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    sender.sendFrame(frame, TimeSpan(600));
    events = second.waitFor(2);
    REQUIRE(events.size() == 2u);
    CHECK(events[0] == "hello cfg text");
    CHECK(events[1] == "frame 600 n=40");

    // The first receiver lost its connection and got nothing more.
    receiver.reset();
    CHECK(first.waitFor(6).size() == 5u);
}

TEST_CASE("A LAN receiver keeps trying while there is no sender, and stops when destroyed") {
    std::mutex gate;
    std::vector<std::string> statuses;
    auto receiver = std::make_unique<LanReceiver>("127.0.0.1", 1);  // nothing listens on port 1
    receiver->statusChanged = [&](bool connected, const std::string& message) {
        std::lock_guard lock(gate);
        statuses.push_back((connected ? "up " : "down ") + message);
    };
    const auto start = [&] {
        receiver->start([](const std::string&) {}, [](const Frame&, TimeSpan) {},
                        [](const FrameMalformedException&, TimeSpan) {},
                        [](std::span<const ImuSample>) {});
    };
    start();
    CHECK_THROWS_AS(start(), Sarwave::Core::InvalidOperationException);
    REQUIRE(waitUntil([&] {
        std::lock_guard lock(gate);
        return !statuses.empty();
    }));

    // Destroyed while waiting to retry: returns at once rather than after the 2 s wait.
    const auto before = std::chrono::steady_clock::now();
    receiver.reset();
    CHECK(std::chrono::steady_clock::now() - before < std::chrono::milliseconds(500));

    std::lock_guard lock(gate);
    CHECK(statuses.front().starts_with("down No connection to 127.0.0.1:1: "));
}
