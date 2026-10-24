#pragma once

#include <atomic>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <exception>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

#include "Imu/Sampling/ImuSampler.hpp"
#include "MMWave/Streaming/ConstantCapture.hpp"

namespace Sarwave::Integration::PeopleTracking::Lan {

/// Receives a session a LanSender streams: connects to it, hands each message to the callbacks
/// given to start(), and connects again whenever the connection is lost or cannot be made, until
/// destroyed.
///
/// All callbacks run on the receiver's own thread, one at a time and in the order the sender
/// sent them, so they can share state without locks. A frame callback may let a
/// FrameMalformedException for its frame escape: the frame then goes to the malformed callback
/// instead, as with a live capture. Any other exception from a callback ends the receiver; see
/// error().
class LanReceiver {
   public:
    using HelloCallback = std::function<void(const std::string& cfgText)>;

    /// host: the sender's IP address or host name.
    LanReceiver(std::string host, int port);

    /// Disconnects and stops, waiting for a callback in progress to return.
    ~LanReceiver();

    LanReceiver(const LanReceiver&) = delete;
    LanReceiver& operator=(const LanReceiver&) = delete;

    /// Called when the connection is made (true) or lost or refused (false), with a line to
    /// show. Called on the receiver's thread. Set before start().
    std::function<void(bool connected, const std::string& message)> statusChanged;

    /// onHello: each new connection's first message, the cfg text the sender's radar runs.
    ///   Anything kept from an earlier connection belongs to another session, on another clock.
    /// onFrame: a frame, on the sender's capture clock. Gone when the callback returns.
    /// onMalformed: a frame the sender, or onFrame, found malformed.
    /// onImu: raw IMU samples, in the IMU's own axes, on the same clock as the frames.
    ///
    /// Throws InvalidOperationException when the receiver has already been started.
    void start(HelloCallback onHello,
               Core::MMWave::Streaming::ConstantCapture::FrameCallback onFrame,
               Core::MMWave::Streaming::ConstantCapture::MalformedCallback onMalformed,
               Core::Imu::Sampling::ImuSampler::BatchCallback onImu);

    /// Whether the receiver has stopped: destroyed, or a callback failed (see error()).
    [[nodiscard]] bool stopped() const {
        return m_finished.load();
    }

    /// The exception from a callback that ended the receiver, if one did. Read once stopped().
    [[nodiscard]] std::exception_ptr error() const {
        return m_error;
    }

   private:
    struct Callbacks;

    void run(const Callbacks& callbacks);
    void receive(const Callbacks& callbacks);
    template <class Initiate>
    void runOperation(Initiate initiate);
    void connect();
    std::size_t read(void* data, std::size_t size, boost::system::error_code& error);
    /// False when the sender closed the connection instead.
    bool readHeader(void* data, std::size_t size);
    void readExactly(void* data, std::size_t size);
    void waitBeforeRetrying();
    void status(bool connected, const std::string& message) const;

    std::string m_host;
    int m_port;

    boost::asio::io_context m_io;
    boost::asio::ip::tcp::resolver m_resolver;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::steady_timer m_timer;
    std::mutex m_gate;
    std::atomic<bool> m_stopping = false;
    std::atomic<bool> m_finished = false;
    std::exception_ptr m_error;
    std::thread m_thread;
};

}  // namespace Sarwave::Integration::PeopleTracking::Lan
