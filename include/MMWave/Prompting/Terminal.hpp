#pragma once
#include <iostream>
#include <ostream>
#include <print>
#include <stdexcept>
#include <string>

#include "General.hpp"

namespace MMWave::Prompting::Terminal {
static void portPrompt(const std::string& main_prompt, int* port_number)
{
    auto call_error = [](std::string message) {
        std::cout << "\033[1A\033[2K\r" << std::flush;
        std::println("  !! {} !!", message);
    };

    std::string str_input;
    int int_input;

    std::println("{}", main_prompt);

    while (true) {
        int_input = 0;
        std::print("> ");

        if (!(std::cin >> str_input)) {
            call_error("Invalid String input!");
            continue;
        }

        try {
            int_input = General::checkStrArgument(str_input);
            break;
        }
        catch (const std::logic_error& e) {
            call_error(e.what());
        }
    }

    *port_number = int_input;
}

inline General::MMWavePortPaths askPorts()
{
    int cli = 0;
    int data = 0;

    portPrompt("Enter the CLI Port (Command / User Control) Enhanced COM Port", &cli);
    portPrompt("Data Port (Auxiliary / Data Stream) Standard COM Port", &data);

    return General::createPath(cli, data);
}
}  // namespace MMWave::Prompting::Terminal
