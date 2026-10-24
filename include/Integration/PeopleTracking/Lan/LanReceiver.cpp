#include "Integration/PeopleTracking/Lan/LanReceiver.hpp"

#include <array>
#include <boost/asio/buffer.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/read.hpp>
#include <chrono>
#include <cstdint>
#include <vector>

#include "Integration/PeopleTracking/Lan/LanFormat.hpp"
#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Exceptions/FrameMalformedException.hpp"
#include "MMWave/Recording/RecordingFormat.hpp"
#include "MMWave/Streaming/Frame.hpp"

namespace Sarwave::Integration::PeopleTracking::Lan {

namespace asio = boost::asio;
using asio::ip::tcp;
using Core::InvalidDataException;
using Core::IOException;
using Core::MMWave::TimeSpan;
using Core::MMWave::Exceptions::FrameMalformedException;

namespace {

constexpr auto RetryDelay = std::chrono::seconds(2);

/// The receiver is being destroyed: whatever was pending failed because of it.
struct Stopping {};

}  // namespace

struct LanReceiver::Callbacks {
    HelloCallback onHello;
    Core::MMWave::Streaming::ConstantCapture::FrameCallback onFrame;
    Core::MMWave::Streaming::ConstantCapture::MalformedCallback onMalformed;
    Core::Imu::Sampling::ImuSampler::BatchCallback onImu;
};

LanReceiver::LanReceiver(std::string host, int port)
    : m_host(std::move(host)), m_port(port), m_resolver(m_io), m_socket(m_io), m_timer(m_io) {
}

LanReceiver::~LanReceiver() {
    {
        std::lock_guard lock(m_gate);
        m_stopping = true;
        asio::post(m_io, [this] {
            boost::system::error_code ignored;
            m_resolver.cancel();
            m_socket.close(ignored);
            m_timer.cancel();
        });
    }
    if (m_thread.joinable()) m_thread.join();
}

void LanReceiver::start(HelloCallback onHello,
                        Core::MMWave::Streaming::ConstantCapture::FrameCallback onFrame,
                        Core::MMWave::Streaming::ConstantCapture::MalformedCallback onMalformed,
                        Core::Imu::Sampling::ImuSampler::BatchCallback onImu) {
    if (m_thread.joinable())
        throw Core::InvalidOperationException("The receiver has already been started.");

    m_thread = std::thread(
        [this, callbacks = Callbacks{std::move(onHello), std::move(onFrame), std::move(onMalformed),
                                     std::move(onImu)}] { run(callbacks); });
}

void LanReceiver::run(const Callbacks& callbacks) {
    const std::string where = m_host + ":" + std::to_string(m_port);
    try {
        while (!m_stopping) {
            try {
                connect();
                status(true, "Connected to " + where + ".");
                receive(callbacks);
                status(false, "The sender at " + where + " closed the connection.");
            } catch (const boost::system::system_error& ex) {
                if (m_stopping) break;
                status(false, "No connection to " + where + ": " + ex.code().message());
            } catch (const IOException& ex) {
                if (m_stopping) break;
                status(false, "No connection to " + where + ": " + ex.what());
            } catch (const InvalidDataException& ex) {
                if (m_stopping) break;
                status(false, "No connection to " + where + ": " + ex.what());
            }

            boost::system::error_code ignored;
            m_socket.close(ignored);
            waitBeforeRetrying();
        }
    } catch (const Stopping&) {
    } catch (const std::exception& ex) {
        if (!m_stopping) {
            m_error = std::current_exception();
            status(false, std::string("Receiving stopped: ") + ex.what());
        }
    } catch (...) {
        if (!m_stopping) {
            m_error = std::current_exception();
            status(false, "Receiving stopped.");
        }
    }
    m_finished = true;
}

void LanReceiver::receive(const Callbacks& callbacks) {
    std::array<std::uint8_t, LanFormat::HeaderSize> header{};
    std::vector<std::uint8_t> payload(64 * 1024);
    std::vector<Core::Imu::ImuSample> samples(64);
    bool greeted = false;

    while (true) {
        if (!readHeader(header.data(), header.size())) return;

        std::uint8_t type = 0;
        std::int32_t length = 0;
        LanFormat::readHeader(header, type, length);
        if (payload.size() < static_cast<std::size_t>(length))
            payload.resize(static_cast<std::size_t>(length));

        readExactly(payload.data(), static_cast<std::size_t>(length));
        const std::span<const std::uint8_t> content(payload.data(),
                                                    static_cast<std::size_t>(length));

        if (type == LanFormat::Hello) {
            callbacks.onHello(LanFormat::decodeHello(content));
            greeted = true;
            continue;
        }

        if (!greeted) throw InvalidDataException("The sender did not say hello first.");

        switch (type) {
            case LanFormat::Frame: {
                const TimeSpan timestamp = LanFormat::readTimestamp(content);
                const auto frame = Core::MMWave::Streaming::Frame::copyFrom(
                    content.subspan(LanFormat::TimestampSize));
                try {
                    callbacks.onFrame(frame, timestamp);
                } catch (const FrameMalformedException& ex) {
                    callbacks.onMalformed(ex, timestamp);
                }
                break;
            }
            case LanFormat::Malformed: {
                const TimeSpan timestamp = LanFormat::readTimestamp(content);
                const auto exception = Core::MMWave::Recording::RecordingFormat::decodeMalformed(
                    content.subspan(LanFormat::TimestampSize));
                callbacks.onMalformed(*exception, timestamp);
                break;
            }
            case LanFormat::Imu: {
                const int count = LanFormat::decodeImu(content, samples);
                callbacks.onImu(std::span<const Core::Imu::ImuSample>(
                    samples.data(), static_cast<std::size_t>(count)));
                break;
            }
            default:
                break;
        }
    }
}

// The blocking operations below each start one asynchronous operation and run the receiver's
// io_context until it completes, so the destructor can end them from another thread: it posts a
// handler that closes the socket and cancels the resolver and the timer. An operation not yet
// started when the destructor runs sees m_stopping instead; m_gate keeps the two from crossing.

template <class Initiate>
void LanReceiver::runOperation(Initiate initiate) {
    {
        std::lock_guard lock(m_gate);
        if (m_stopping) throw Stopping{};
        initiate();
    }
    m_io.restart();
    m_io.run();
    if (m_stopping) throw Stopping{};
}

void LanReceiver::connect() {
    boost::system::error_code error;
    tcp::resolver::results_type endpoints;
    runOperation([&] {
        m_resolver.async_resolve(
            m_host, std::to_string(m_port),
            [&](const boost::system::error_code& e, tcp::resolver::results_type r) {
                error = e;
                endpoints = std::move(r);
            });
    });
    if (error) throw boost::system::system_error(error);

    runOperation([&] {
        asio::async_connect(
            m_socket, endpoints,
            [&](const boost::system::error_code& e, const tcp::endpoint&) { error = e; });
    });
    if (error) throw boost::system::system_error(error);

    m_socket.set_option(tcp::no_delay(true), error);
}

std::size_t LanReceiver::read(void* data, std::size_t size, boost::system::error_code& error) {
    std::size_t count = 0;
    runOperation([&] {
        asio::async_read(m_socket, asio::buffer(data, size),
                         [&](const boost::system::error_code& e, std::size_t n) {
                             error = e;
                             count = n;
                         });
    });
    return count;
}

bool LanReceiver::readHeader(void* data, std::size_t size) {
    boost::system::error_code error;
    read(data, size, error);
    // The sender closed the connection.
    if (error == asio::error::eof) return false;
    if (error) throw boost::system::system_error(error);
    return true;
}

void LanReceiver::readExactly(void* data, std::size_t size) {
    if (size == 0) return;
    boost::system::error_code error;
    read(data, size, error);
    if (error == asio::error::eof)
        throw IOException("Unable to read beyond the end of the stream.");
    if (error) throw boost::system::system_error(error);
}

void LanReceiver::waitBeforeRetrying() {
    runOperation([&] {
        m_timer.expires_after(RetryDelay);
        m_timer.async_wait([](const boost::system::error_code&) {});
    });
}

void LanReceiver::status(bool connected, const std::string& message) const {
    if (statusChanged) statusChanged(connected, message);
}

}  // namespace Sarwave::Integration::PeopleTracking::Lan
