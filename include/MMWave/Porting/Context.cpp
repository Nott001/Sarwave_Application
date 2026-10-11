#include "MMWave/Porting/Context.hpp"

#include <algorithm>
#include <boost/asio.hpp>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "MMWave/Common/Exceptions.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#endif

namespace Sarwave::Core::MMWave::Porting {
namespace {

namespace asio = boost::asio;
using Clock = std::chrono::steady_clock;
using std::chrono::milliseconds;

/// How long a blocked data-port read waits between checks of its stop token.
constexpr milliseconds StopPollInterval{50};

/// The CLI port's per-byte ReadTimeout in the C# version.
constexpr milliseconds CliReadTimeout{200};

/// One serial port with its own io_context, so the CLI and data ports can be driven from
/// different threads without sharing an event loop.
struct Port {
    asio::io_context io;
    asio::serial_port port{io};

    void open(const std::string& path, unsigned int baud) {
        std::string device = path;
#ifdef _WIN32
        // COM10 and above are only reachable through the device namespace.
        if (device.rfind("COM", 0) == 0) device = "\\\\.\\" + device;
#endif
        boost::system::error_code ec;
        port.open(device, ec);
        if (ec) throw IOException(openErrorMessage(path, ec));

        using base = asio::serial_port_base;
        port.set_option(base::baud_rate(baud));
        port.set_option(base::character_size(8));
        port.set_option(base::parity(base::parity::none));
        port.set_option(base::stop_bits(base::stop_bits::one));
        port.set_option(base::flow_control(base::flow_control::none));
    }

    /// The messages .NET's SerialPort.Open gives for the common failures.
    static std::string openErrorMessage(const std::string& path,
                                        const boost::system::error_code& ec) {
        if (ec == boost::system::errc::no_such_file_or_directory) {
            return "Could not find file '" + path + "'.";
        }
        if (ec == boost::system::errc::permission_denied) {
            return "Access to the path '" + path + "' is denied.";
        }
        return ec.message();
    }

    void discardInBuffer() {
#ifdef _WIN32
        PurgeComm(port.native_handle(), PURGE_RXCLEAR);
#else
        tcflush(port.native_handle(), TCIFLUSH);
#endif
    }

    enum class ReadOutcome {
        Data,
        TimedOut,
        Stopped,
    };

    /// One async_read_some, driven on the calling thread until it completes, `timeout`
    /// passes, or `stop` is requested. A pending read is always cancelled and drained
    /// before returning, so nothing is left referring to this call's locals.
    ReadOutcome readSome(std::span<std::uint8_t> buffer, std::optional<milliseconds> timeout,
                         const std::stop_token& stop, std::size_t& bytesRead,
                         boost::system::error_code& error) {
        bool done = false;
        port.async_read_some(asio::buffer(buffer.data(), buffer.size()),
                             [&](const boost::system::error_code& ec, std::size_t n) {
                                 error = ec;
                                 bytesRead = n;
                                 done = true;
                             });

        io.restart();
        const auto deadline = timeout ? Clock::now() + *timeout : Clock::time_point::max();
        bool expired = false;
        bool stopped = false;
        while (!done) {
            const auto now = Clock::now();
            if (now >= deadline) {
                expired = true;
                break;
            }
            if (stop.stop_requested()) {
                stopped = true;
                break;
            }
            const auto slice = std::min<Clock::duration>(StopPollInterval, deadline - now);
            io.run_one_for(slice);
        }

        if (!done) {
            boost::system::error_code ignored;
            port.cancel(ignored);
            while (!done) {
                io.restart();
                io.run_one();
            }
            // Bytes may have arrived in the meantime; they count as a normal read.
            if (!error && bytesRead > 0) return ReadOutcome::Data;
            return stopped ? ReadOutcome::Stopped : ReadOutcome::TimedOut;
        }
        (void)expired;
        return ReadOutcome::Data;
    }

    void close() {
        boost::system::error_code ignored;
        if (port.is_open()) {
            port.cancel(ignored);
            port.close(ignored);
        }
    }
};

std::string trimEnd(std::string text, char c) {
    while (!text.empty() && text.back() == c) text.pop_back();
    return text;
}

bool equalsIgnoreCase(std::string_view a, std::string_view b) {
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](char x, char y) {
               const auto lower = [](char c) {
                   return c >= 'A' && c <= 'Z' ? char(c - 'A' + 'a') : c;
               };
               return lower(x) == lower(y);
           });
}

/// File.ReadLines: UTF-8 (a BOM is dropped), lines split on "\n", "\r\n" or a lone "\r".
std::vector<std::string> readLines(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) throw IOException("Could not open cfg file: " + path);

    std::ostringstream content;
    content << file.rdbuf();
    std::string text = content.str();
    if (text.rfind("\xEF\xBB\xBF", 0) == 0) text.erase(0, 3);

    std::vector<std::string> lines;
    std::string current;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        if (c == '\n' || c == '\r') {
            lines.push_back(std::move(current));
            current.clear();
            if (c == '\r' && i + 1 < text.size() && text[i + 1] == '\n') ++i;
        } else {
            current += c;
        }
    }
    if (!current.empty()) lines.push_back(std::move(current));
    return lines;
}

}  // namespace

struct Context::Impl {
    Port cli;
    Port data;
};

std::unique_ptr<Context> Context::openMmwavePorts(const MmwavePortPaths& ports) {
    return openMmwavePorts(ports.cli, ports.data);
}

std::unique_ptr<Context> Context::openMmwavePorts(const std::string& cliPort,
                                                  const std::string& dataPort) {
    return std::unique_ptr<Context>(new Context(cliPort, dataPort));
}

Context::Context(const std::string& cliPath, const std::string& dataPath)
    : m_impl(std::make_unique<Impl>()) {
    try {
        m_impl->cli.open(cliPath, 115200);
        m_impl->data.open(dataPath, 921600);
        // Drop stale frames if the sensor was left running from a prior session.
        m_impl->data.discardInBuffer();
    } catch (...) {
        dispose();  // don't leak an already-open port
        throw;
    }
}

Context::~Context() {
    dispose();
}

std::string Context::getVersion() {
    return sendCliCommand("version");
}

bool Context::validateCliPort() {
    return getVersion().find(VersionMarker) != std::string::npos;
}

std::string Context::sendCliCommand(std::string_view command, std::optional<milliseconds> timeout) {
    if (!m_impl) throw ObjectDisposedException("SerialPort");

    const milliseconds limit = timeout.value_or(std::chrono::seconds(2));
    const std::string toSend = std::string(command) + "\n";
    boost::system::error_code writeError;
    asio::write(m_impl->cli.port, asio::buffer(toSend), writeError);
    if (writeError) throw IOException(writeError.message());

    std::string response;
    const auto start = Clock::now();
    std::uint8_t byte = 0;

    while (Clock::now() - start < limit) {
        std::size_t n = 0;
        boost::system::error_code error;
        const auto outcome = m_impl->cli.readSome({&byte, 1}, CliReadTimeout, {}, n, error);
        if (outcome == Port::ReadOutcome::TimedOut) {
            // No byte within ReadTimeout; loop re-checks the overall limit.
            continue;
        }
        if (error || n == 0) break;

        response += static_cast<char>(byte);
        if (byte == 'e' && response.ends_with("Done")) break;
    }
    return response;
}

void Context::sendConfigFile(const std::string& cfgPath, const ReportLine& reportLine,
                             bool skipSensorStart) {
    if (!std::filesystem::is_regular_file(cfgPath)) {
        throw FileNotFoundException("Could not open cfg file: " + cfgPath);
    }

    for (const std::string& raw : readLines(cfgPath)) {
        const std::string line = trimEnd(raw, '\r');
        if (line.empty() || line[0] == '%') continue;

        if (skipSensorStart && isSensorStartCommand(line)) continue;

        const std::string response = sendCliCommand(line);
        if (reportLine) reportLine(line, response);

        if (response.find("Done") == std::string::npos) {
            throw InvalidOperationException("Config command failed or timed out: " + line);
        }
    }
}

bool Context::isSensorStartCommand(std::string_view line) {
    // Matches on the command word only (not the whole line), so this still catches
    // "sensorStart 0" -- some SDK versions accept a doReconfig argument.
    const auto end = line.find(' ');
    const std::string_view command = end == std::string_view::npos ? line : line.substr(0, end);
    return equalsIgnoreCase(command, SensorStartCommand);
}

std::size_t Context::readData(std::span<std::uint8_t> buffer, std::stop_token stop) {
    if (!m_impl) throw ObjectDisposedException("SerialPort");

    std::size_t n = 0;
    boost::system::error_code error;
    const auto outcome = m_impl->data.readSome(buffer, std::nullopt, stop, n, error);
    if (outcome == Port::ReadOutcome::Stopped) throw OperationCanceledException();

    if (error == asio::error::eof) return 0;
    if (error) throw std::runtime_error(error.message());
    return n;
}

void Context::dispose() {
    if (!m_impl) return;
    m_impl->cli.close();
    m_impl->data.close();
    m_impl.reset();
}

}  // namespace Sarwave::Core::MMWave::Porting
