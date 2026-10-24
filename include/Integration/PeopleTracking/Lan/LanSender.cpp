#include "Integration/PeopleTracking/Lan/LanSender.hpp"

#include <array>
#include <boost/asio/buffer.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/write.hpp>
#include <sstream>

#include "Integration/PeopleTracking/Lan/LanFormat.hpp"

namespace Sarwave::Integration::PeopleTracking::Lan {

namespace asio = boost::asio;
using asio::ip::tcp;

namespace {

std::string toString(const tcp::endpoint& endpoint) {
    std::ostringstream text;
    text << endpoint;
    return text.str();
}

}  // namespace

LanSender::LanSender(const asio::ip::address& address, int port, std::string cfgText, int maxQueued)
    : m_acceptor(m_io),
      m_endpoint(address, static_cast<unsigned short>(port)),
      m_cfgText(std::move(cfgText)),
      m_maxQueued(static_cast<std::size_t>(maxQueued)) {
}

LanSender::~LanSender() {
    if (!m_thread.joinable()) return;
    // With the acceptor and the receiver closed, their pending operations end and the thread's
    // run() returns.
    asio::post(m_io, [this] {
        boost::system::error_code ignored;
        m_acceptor.close(ignored);  // ends the pending accept
        if (m_client) m_client->close(ignored);
        m_client.reset();
        m_connected = false;
    });
    m_thread.join();
}

void LanSender::start() {
    m_acceptor.open(m_endpoint.protocol());
    m_acceptor.set_option(tcp::acceptor::reuse_address(true));
    m_acceptor.bind(m_endpoint);
    m_acceptor.listen(1);
    m_port = m_acceptor.local_endpoint().port();
    accept();
    m_thread = std::thread([this] { m_io.run(); });
    say("Listening on " + toString(m_acceptor.local_endpoint()) + ".");
}

void LanSender::sendFrame(const Core::MMWave::Streaming::Frame& frame,
                          Core::MMWave::TimeSpan timestamp) {
    if (isConnected()) enqueue(LanFormat::encodeFrame(frame.bytes(), timestamp));
}

void LanSender::sendMalformed(const Core::MMWave::Exceptions::FrameMalformedException& exception,
                              Core::MMWave::TimeSpan timestamp) {
    if (isConnected()) enqueue(LanFormat::encodeMalformed(exception, timestamp));
}

void LanSender::sendImu(std::span<const Core::Imu::ImuSample> samples) {
    if (isConnected() && !samples.empty()) enqueue(LanFormat::encodeImu(samples));
}

void LanSender::enqueue(std::vector<std::uint8_t> message) {
    {
        std::lock_guard lock(m_gate);
        if (m_queue.size() >= m_maxQueued) {
            ++m_dropped;
            return;
        }
        m_queue.push_back(std::move(message));
    }
    asio::post(m_io, [this] { writeNext(); });
}

void LanSender::accept() {
    auto socket = std::make_shared<Socket>(m_io);
    m_acceptor.async_accept(*socket, [this, socket](const boost::system::error_code& error) {
        if (error == asio::error::operation_aborted || !m_acceptor.is_open()) return;
        if (error) {
            say("Accepting a receiver failed: " + error.message());
        } else {
            boost::system::error_code ignored;
            socket->set_option(tcp::no_delay(true), ignored);
            say("Receiver connected from " + toString(socket->remote_endpoint(ignored)) + ".");

            // A receiver that reconnects (restarted, or its old connection silently died)
            // replaces the old one rather than waiting behind it.
            if (m_client) m_client->close(ignored);
            m_client = socket;
            m_greeted = false;
            m_connected = true;
        }
        accept();
    });
}

void LanSender::writeNext() {
    if (m_writing) return;

    // Without a receiver a message goes nowhere: it was queued for one that has left.
    do {
        std::lock_guard lock(m_gate);
        if (m_queue.empty()) return;
        m_current = std::move(m_queue.front());
        m_queue.pop_front();
    } while (!m_client);

    std::array<asio::const_buffer, 2> buffers{asio::buffer(m_current), asio::const_buffer()};
    if (!m_greeted) {
        m_hello = LanFormat::encodeHello(m_cfgText);
        buffers = {asio::buffer(m_hello), asio::buffer(m_current)};
        m_greeted = true;
    }

    m_writing = true;
    auto client = m_client;
    asio::async_write(*client, buffers,
                      [this, client](const boost::system::error_code& error, std::size_t) {
                          m_writing = false;
                          // Only forget the receiver if no newer one has replaced it meanwhile.
                          if (error && client == m_client) {
                              boost::system::error_code ignored;
                              client->close(ignored);
                              m_client.reset();
                              m_connected = false;
                              say("Receiver disconnected: " + error.message());
                          }
                          writeNext();
                      });
}

void LanSender::say(const std::string& line) const {
    if (log) log(line);
}

}  // namespace Sarwave::Integration::PeopleTracking::Lan
