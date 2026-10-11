#include "MMWave/Prompting/Prompting.hpp"

#include <iostream>

#include "MMWave/Common/DotNetFormat.hpp"
#include "MMWave/Common/Exceptions.hpp"

namespace Sarwave::Core::MMWave::Prompting {

namespace General {

std::string defaultPortDirectory() {
#if defined(_WIN32)
    return "COM";
#elif defined(__linux__)
    return "/dev/ttyUSB";
#else
    throw NotSupportedException("Ports are not supported for this OS");
#endif
}

int checkStrArgument(const std::string& strInput) {
    std::int32_t intInput = 0;
    if (!DotNet::tryParseInt32(strInput, intInput)) {
        throw ArgumentException("Failed to parse integer from input '" + strInput + "'");
    }

    if (intInput < 0) {
        // The C# code passes this text as ArgumentOutOfRangeException's *parameter name*, so
        // .NET wraps it in its stock message; reproduced as is.
        throw ArgumentOutOfRangeException(
            "Port number cannot be negative (received: " + std::to_string(intInput) + ")");
    }

    return intInput;
}

namespace {
std::string buildPortName(int portNumber) {
    return defaultPortDirectory() + std::to_string(portNumber);
}
}  // namespace

Porting::MmwavePortPaths createPath(int cli, int data) {
    return {buildPortName(cli), buildPortName(data)};
}

}  // namespace General

namespace Terminal {

void portPrompt(const std::string& mainPrompt, int& portNumber) {
    while (true) {
        std::cout << mainPrompt << "\n";
        std::cout << "> " << std::flush;

        std::string strInput;
        if (!std::getline(std::cin, strInput)) {
            // Console.ReadLine() returned null (end of input).
            std::cin.clear();
            std::cout << "  !! Invalid String input !!\n";
            continue;
        }
        if (!strInput.empty() && strInput.back() == '\r') strInput.pop_back();

        try {
            portNumber = General::checkStrArgument(strInput);
            break;
        } catch (const ArgumentException& e) {
            std::cout << "  !! " << e.what() << " !!\n";
        }
    }
}

Porting::MmwavePortPaths askPorts() {
    int cli = 0;
    int data = 0;
    portPrompt("Enter the CLI Port (Command / User Control) Enhanced COM Port", cli);
    portPrompt("Data Port (Auxiliary / Data Stream) Standard COM Port", data);
    return General::createPath(cli, data);
}

}  // namespace Terminal
}  // namespace Sarwave::Core::MMWave::Prompting
