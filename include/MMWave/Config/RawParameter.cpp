#include "MMWave/Config/RawParameter.hpp"

namespace Sarwave::Core::MMWave::Config {

std::string RawParameter::toString() const {
    std::string line = m_paramName;
    for (const std::string& argument : m_arguments) {
        line += ' ';
        line += argument;
    }

    return line;
}

}  // namespace Sarwave::Core::MMWave::Config
