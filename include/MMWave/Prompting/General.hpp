#pragma once
#include <stdexcept>
#include <string>
#include <format>

namespace MMWave::Prompting::General {
    struct MMWavePortPaths {
        std::string cli;
        std::string data;
    };

    inline int checkStrArgument(const std::string &str_input) {
        int int_input = 0;

        try {
            int_input = std::stoi(str_input);
        } catch (const std::invalid_argument &) {
            throw std::invalid_argument(
                std::format("Failed to parse integer from input '{}'", str_input)
            );
        } catch (const std::out_of_range &) {
            throw std::out_of_range(
                std::format("Integer value in '{}' exceeds numerical range limits", str_input)
            );
        }

        if (int_input < 0)
            throw std::out_of_range(
                "Port number cannot be negative (received: " + std::to_string(int_input) + ")"
                );

        return int_input;
    }

    inline std::string buildPortName(const int portNumber) {
#ifdef _WIN32
        return std::format("COM{}", portNumber);
#else
        return std::format("/dev/ttyUSB{}", portNumber);
#endif
    }

    inline MMWavePortPaths createPath(const int cli, const int data) {
        return MMWavePortPaths(
            buildPortName(cli),
            buildPortName(data)
            );
    }
}