#include <print>
#include <boost/asio.hpp>
#include <iostream>
#include <limits>

#include "MMWave/Prompting/Terminal.hpp"
#include "MMWave/Porter.hpp"
#include "MMWave/Streaming.hpp"
#include "MMWave/Tlv.hpp"

using boost::asio::serial_port_base;

int main() {
    const auto port_paths = MMWave::Prompting::Terminal::askPorts();
    auto mmwave = MMWave::Porter::openMMWavePorts(port_paths);

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::println("");

    std::println("Location of the config file: ");
    std::string cfgPathInput;
    std::getline(std::cin, cfgPathInput);

    std::println("");

    MMWave::Porter::sendConfigFile(
        mmwave,
        cfgPathInput,
        [](const std::string& line, const std::string& response) {
            std::println("-> {}", line);

            std::string cleaned = response;
            std::erase(cleaned, '\r');

            std::istringstream responseStream(cleaned);
            std::string response_line;
            bool first = true;
            while (std::getline(responseStream, response_line)) {
                if (response_line.empty() && first) continue;
                std::println("   : {}", response_line);
                first = false;
            }
        }
        );

    std::println("");

    std::string version = MMWave::Porter::getVersion(mmwave);
    std::println("{}", version);
    if (version.find("mmWave SDK Version") == std::string::npos) {
        throw std::runtime_error("CLI port validation failed");
    }

    std::println("");

    MMWave::Streaming::FrameReader reader(mmwave);

    while (true) {
        try {
            MMWave::Streaming::Frame frame = reader.readNextFrame();

            std::println("Frame #{}: {} objects", frame.header.frameNumber, frame.header.numDetectedObj);

            for (const auto& tlv : MMWave::Tlv::TlvRange(frame)) {
                if (tlv.type != MMWave::Tlv::TLV_DETECTED_POINTS) continue;

                for (const auto& point : tlv.points()) {
                    std::println("  x={:.3f} y={:.3f} z={:.3f} v={:.3f}",
                                 point.x, point.y, point.z, point.velocity);
                }
            }
        } catch (const std::exception& e) {
            std::println("Stream ended: {}", e.what());
            break;
        }
    }

    return 0;
}