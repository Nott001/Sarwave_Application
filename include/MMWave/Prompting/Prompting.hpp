#pragma once

#include <string>

#include "MMWave/Porting/Context.hpp"

namespace Sarwave::Core::MMWave::Prompting {

/// Port naming shared by the terminal and GUI prompts.
namespace General {

/// "COM" on Windows, "/dev/ttyUSB" on Linux; throws NotSupportedException elsewhere.
std::string defaultPortDirectory();

/// Parses a port number. Throws ArgumentException ("Failed to parse integer from input '...'")
/// or ArgumentOutOfRangeException for a negative number, with the C# version's messages.
int checkStrArgument(const std::string& strInput);

/// The two device paths for port numbers `cli` and `data`, e.g. /dev/ttyUSB0 and /dev/ttyUSB1.
Porting::MmwavePortPaths createPath(int cli, int data);

}  // namespace General

/// Console prompts.
namespace Terminal {

/// Asks for a port number until a valid one is entered.
void portPrompt(const std::string& mainPrompt, int& portNumber);

/// Asks for the CLI and data port numbers.
Porting::MmwavePortPaths askPorts();

}  // namespace Terminal
}  // namespace Sarwave::Core::MMWave::Prompting
