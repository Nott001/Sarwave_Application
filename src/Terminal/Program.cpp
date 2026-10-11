// Sarwave.Source.Terminal: opens the radar's ports, sends a .cfg file, starts the sensor and
// captures frames until the process is killed.
//
// Usage (argument positions exactly as the C# Program.Main reads them -- note args[0] is
// unused, and args[2] is read both as the data port and as the cfg path):
//   SarwaveTerminal                          prompts for port numbers and the cfg path
//   SarwaveTerminal <x> <cliPort> <dataPort> uses those port paths, cfg path = <dataPort>

#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Porting/Context.hpp"
#include "MMWave/Prompting/Prompting.hpp"
#include "MMWave/Streaming/ConstantCapture.hpp"

#ifdef _WIN32
#include <conio.h>
#include <io.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace {

using namespace Sarwave::Core;
using namespace Sarwave::Core::MMWave;

/// Console.ReadKey(): waits for one key press (no Enter needed) and echoes it.
void readKey() {
#ifdef _WIN32
    if (!_isatty(_fileno(stdin))) {
        throw InvalidOperationException(
            "Cannot read keys when either application does not have a console or when console "
            "input has been "
            "redirected. Try Console.Read.");
    }
    const int ch = _getch();
    std::cout << static_cast<char>(ch) << std::flush;
#else
    if (!isatty(STDIN_FILENO)) {
        throw InvalidOperationException(
            "Cannot read keys when either application does not have a console or when console "
            "input has been "
            "redirected. Try Console.Read.");
    }
    termios original{};
    tcgetattr(STDIN_FILENO, &original);
    termios raw = original;
    raw.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    char ch = 0;
    const auto n = read(STDIN_FILENO, &ch, 1);
    tcsetattr(STDIN_FILENO, TCSANOW, &original);
    if (n == 1 && ch != '\n') std::cout << ch << std::flush;
#endif
}

/// Splits a CLI reply into lines the way StringReader.ReadLine does.
std::vector<std::string> readLines(const std::string& text) {
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

int run(const std::vector<std::string>& args) {
    const auto clockStart = std::chrono::steady_clock::now();
    (void)clockStart;

    Porting::MmwavePortPaths ports;
    if (args.size() > 2) {
        ports = {args[1], args[2]};
    } else {
        ports = Prompting::Terminal::askPorts();
    }

    const auto mmwaveCtx = Porting::Context::openMmwavePorts(ports);

    const std::string version = mmwaveCtx->getVersion();
    std::cout << version << "\n";
    if (version.find("mmWave SDK Version") == std::string::npos) {
        throw InvalidOperationException("CLI port validation failed");
    }

    std::string cfgPathInput;
    if (args.size() >= 3) {
        cfgPathInput = args[2];
    } else {
        std::cout << "\n";
        std::cout << "Location of the config file: \n";
        std::getline(std::cin, cfgPathInput);
        if (!cfgPathInput.empty() && cfgPathInput.back() == '\r') cfgPathInput.pop_back();
        std::cout << "\n";
    }

    mmwaveCtx->sendConfigFile(
        cfgPathInput,
        [](const std::string& line, const std::string& response) {
            std::cout << "-> " << line << "\n";

            std::string cleaned;
            for (char c : response) {
                if (c != '\r') cleaned += c;
            }

            bool first = true;
            for (const std::string& responseLine : readLines(cleaned)) {
                if (responseLine.empty() && first) continue;
                std::cout << "   : " << responseLine << "\n";
                first = false;
            }
        },
        true);

    std::cout << "\n";

    std::cout << "\n";
    std::cout << "Press any key to proceed into ts.\n" << std::flush;
    readKey();
    std::cout << "\n";

    Streaming::ConstantCapture cc(*mmwaveCtx);

    const auto sw = std::chrono::steady_clock::now();
    mmwaveCtx->sendCliCommand(Porting::Context::SensorStartCommand);
    cc.beginConstantCapture([sw](const Streaming::Frame&) {
        const TimeSpan elapsed = TimeSpans::from(std::chrono::steady_clock::now() - sw);
        (void)elapsed;
        // DebugLogger.PrintFrameDataVerbose(frame, elapsed);
    });

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    // C#'s Main(string[] args) does not include the program name.
    const std::vector<std::string> args(argv + 1, argv + argc);
    try {
        return run(args);
    } catch (...) {
        Sarwave::Core::failUnhandled(std::current_exception());
    }
}
