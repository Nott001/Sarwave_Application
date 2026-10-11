#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <stop_token>
#include <string>
#include <string_view>

namespace Sarwave::Core::MMWave::Porting {

/// The two serial devices of an mmWave EVM: the CLI port (commands in, replies out) and the
/// data port (frames out).
struct MmwavePortPaths {
    std::string cli;
    std::string data;
};

/// The open CLI (115200 baud) and data (921600 baud) ports of one device, 8N1, no handshake.
///
/// Thread use, as in the C# version: the CLI port is used by one caller at a time; the data
/// port is read by one reader at a time (ConstantCapture's thread). Stop the capture before
/// disposing the context.
class Context {
   public:
    static constexpr std::string_view SensorStartCommand = "sensorStart";
    static constexpr std::string_view SensorStopCommand = "sensorStop";

    static constexpr std::string_view VersionMarker = "mmWave SDK Version";

    /// Opens both ports and discards anything already waiting on the data port (stale frames
    /// from a sensor left running by a previous session). Throws IOException if either port
    /// can't be opened (nothing is left open).
    static std::unique_ptr<Context> openMmwavePorts(const MmwavePortPaths& ports);
    static std::unique_ptr<Context> openMmwavePorts(const std::string& cliPort,
                                                    const std::string& dataPort);

    ~Context();
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    /// Sends "version" on the CLI port and returns the raw reply.
    std::string getVersion();

    /// Confirms the CLI port is talking to an mmWave device by sending "version" and checking
    /// the reply for the expected content.
    bool validateCliPort();

    /// Writes `command` + "\n" to the CLI port, then collects the reply until it ends with
    /// "Done" or `timeout` (default 2 s) passes. Returns whatever arrived (bytes as chars).
    std::string sendCliCommand(std::string_view command,
                               std::optional<std::chrono::milliseconds> timeout = std::nullopt);

    using ReportLine = std::function<void(const std::string& line, const std::string& response)>;

    /// Sends every command line of a .cfg file, in order, each after the previous one was
    /// acknowledged. Comment ('%') and blank lines are skipped. Throws FileNotFoundException
    /// for a missing file and InvalidOperationException for a command not answered "Done".
    ///
    /// skipSensorStart: when true, any "sensorStart" line in the cfg is not sent -- lets the
    /// caller finish other setup (UI, starting a ConstantCapture, etc.) before actually
    /// starting the sensor. Send it yourself afterward via sendCliCommand("sensorStart").
    void sendConfigFile(const std::string& cfgPath, const ReportLine& reportLine = {},
                        bool skipSensorStart = false);

    /// Reads whatever the data port has (at least one byte), blocking until something
    /// arrives. Returns 0 at end of stream. Throws OperationCanceledException when `stop` is
    /// requested first, or std::runtime_error when the port fails.
    std::size_t readData(std::span<std::uint8_t> buffer, std::stop_token stop);

    /// Closes both ports. Called by the destructor; a second call does nothing.
    void dispose();

   private:
    struct Impl;

    Context(const std::string& cliPath, const std::string& dataPath);

    static bool isSensorStartCommand(std::string_view line);

    std::unique_ptr<Impl> m_impl;
};

}  // namespace Sarwave::Core::MMWave::Porting
