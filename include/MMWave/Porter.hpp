#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <termios.h>
#endif
#include "Prompting/General.hpp"


namespace MMWave::Porter {
    using boost::asio::serial_port_base;

    struct Context {
        // Shared so the Context stays movable: io_context itself cannot be
        // moved, but the cli/data ports keep referencing the same underlying
        // executor service through the shared object.
        std::shared_ptr<boost::asio::io_context> io;
        boost::asio::serial_port cli;
        boost::asio::serial_port data;

        // Custom constructor to bind ports to our internal io_context
        Context(const std::string& cli_path, const std::string& data_path)
            : io(std::make_shared<boost::asio::io_context>()),
              cli(*io, cli_path),
              data(*io, data_path)
        {
            configurePort(cli, 115200);
            configurePort(data, 921600);

            // Discard any bytes already queued in the OS receive buffer for
            // the Data port -- if the sensor was left running from a prior
            // session, stale frames would otherwise be read first.
#ifdef _WIN32
            PurgeComm(data.native_handle(), PURGE_RXCLEAR);
#else
            tcflush(data.native_handle(), TCIFLUSH);
#endif
        }

    private:
        static void configurePort(boost::asio::serial_port& port, unsigned int baud) {
            port.set_option(serial_port_base::baud_rate(baud));
            port.set_option(serial_port_base::character_size(8));
            port.set_option(serial_port_base::parity(serial_port_base::parity::none));
            port.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
            port.set_option(serial_port_base::flow_control(serial_port_base::flow_control::none));
        }
    };

    inline Context openMMWavePorts(const std::string &cli_port, const std::string &data_port) {
        return {cli_port, data_port};
    }

    inline Context openMMWavePorts(const Prompting::General::MMWavePortPaths &ports) {
        return openMMWavePorts(ports.cli, ports.data);
    }

    // Sends a command on the CLI port, reads back the response until "Done"
    // appears (or the timeout elapses), and returns whatever came back.
    inline std::string sendCliCommand(Context& ctx,
                                       const std::string& command,
                                       const std::chrono::milliseconds timeout = std::chrono::seconds(2)) {
        std::string toSend = command + "\n";
        boost::asio::write(ctx.cli, boost::asio::buffer(toSend));

        std::string response;
        const auto start = std::chrono::steady_clock::now();
        char ch;
        boost::system::error_code ec;

        while (std::chrono::steady_clock::now() - start < timeout) {
            std::size_t n = ctx.cli.read_some(boost::asio::buffer(&ch, 1), ec);
            if (ec) break;
            if (n > 0) {
                response += ch;
                if (response.find("Done") != std::string::npos) break;
            }
        }
        return response;
    }

    // Reads a .cfg file and sends each valid command line over the CLI port,
    // in order, waiting for a response after each before sending the next.
    inline void sendConfigFile(Context& ctx,
                                const std::filesystem::path& cfgPath,
                                void (*reportLine)(const std::string& line, const std::string& response) = nullptr) {
        std::ifstream cfgFile(cfgPath);
        if (!cfgFile.is_open()) {
            throw std::runtime_error("Could not open cfg file: " + cfgPath.string());
        }

        std::string line;
        while (std::getline(cfgFile, line)) {
            // Strip trailing carriage return, in case the file has
            // Windows-style line endings.
            if (!line.empty() && line.back() == '\r') line.pop_back();

            // Skip blank lines and comment lines (the cfg's "% ..." header).
            if (line.empty() || line[0] == '%') continue;

            std::string response = sendCliCommand(ctx, line);

            if (reportLine) reportLine(line, response);

            if (response.find("Done") == std::string::npos) {
                throw std::runtime_error("Config command failed or timed out: " + line);
            }
        }
    }

    inline std::string getVersion(Context& ctx) {
        return sendCliCommand(ctx, "version");
    }

    // Confirms the CLI port is actually talking to an mmWave device by
    // sending "version" and checking the reply contains expected content.
    inline bool validateCliPort(Context& ctx) {
        return getVersion(ctx).find("mmWave SDK Version") != std::string::npos;
    }
}