#pragma once

#include <atomic>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <thread>
#include <vector>

#include "Imu/ImuSample.hpp"
#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Exceptions/FrameMalformedException.hpp"
#include "MMWave/Streaming/Frame.hpp"

namespace Sarwave::Integration::PeopleTracking::Lan {

/// Streams a live session over the LAN to a LanReceiver: every frame as the radar sent it,
/// malformed-frame markers, and the raw IMU samples. Listens on a TCP port and serves one
/// receiver at a time; a receiver that connects while another is served takes its place.
///
/// The send functions never block and never throw, so they can be called straight from the
/// capture and IMU threads. They copy what they are given and queue it for a thread of the
/// sender's own; without a receiver they do nothing, and when the network falls behind, messages
/// that no longer fit in the queue are dropped and counted in dropped().
///
/// Destroying the sender stops it.
class LanSender {
   public:
    /// The port both sides use unless told otherwise.
    static constexpr int DefaultPort = 5577;

    /// address: the local address to listen on; 0.0.0.0 for every interface.
    /// cfgText: the cfg the radar runs, sent to each receiver so it can place targets.
    /// maxQueued: messages held while the network catches up; about 10 s of a session at 256.
    LanSender(const boost::asio::ip::address& address, int port, std::string cfgText,
              int maxQueued = 256);
    ~LanSender();

    LanSender(const LanSender&) = delete;
    LanSender& operator=(const LanSender&) = delete;

    /// Connections, disconnections and errors, as lines to show or log. Called on the sender's
    /// thread. Set before start().
    std::function<void(const std::string&)> log;

    /// Whether a receiver is connected.
    [[nodiscard]] bool isConnected() const {
        return m_connected.load();
    }

    /// The port listened on once started; the one the system picked if 0 was asked for.
    [[nodiscard]] int port() const {
        return m_port;
    }

    /// Messages dropped because the queue was full.
    [[nodiscard]] std::int64_t dropped() const {
        return m_dropped.load();
    }

    /// Starts listening. Throws boost::system::system_error when the address or port cannot be
    /// listened on.
    void start();

    void sendFrame(const Core::MMWave::Streaming::Frame& frame, Core::MMWave::TimeSpan timestamp);
    void sendMalformed(const Core::MMWave::Exceptions::FrameMalformedException& exception,
                       Core::MMWave::TimeSpan timestamp);

    /// Raw samples, as an ImuSession hands them over.
    void sendImu(std::span<const Core::Imu::ImuSample> samples);

   private:
    using Socket = boost::asio::ip::tcp::socket;

    void enqueue(std::vector<std::uint8_t> message);
    void accept();
    void writeNext();
    void say(const std::string& line) const;

    boost::asio::io_context m_io;
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::endpoint m_endpoint;
    std::string m_cfgText;
    std::size_t m_maxQueued;
    int m_port = 0;

    // Touched only on the sender's thread.
    std::shared_ptr<Socket> m_client;
    bool m_greeted = false;
    bool m_writing = false;
    std::vector<std::uint8_t> m_hello;
    std::vector<std::uint8_t> m_current;

    std::mutex m_gate;
    std::deque<std::vector<std::uint8_t>> m_queue;  // guarded by m_gate

    std::atomic<bool> m_connected = false;
    std::atomic<std::int64_t> m_dropped = 0;
    std::thread m_thread;
};

}  // namespace Sarwave::Integration::PeopleTracking::Lan
