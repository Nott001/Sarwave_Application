#pragma once

#include <string>

#include "MMWave/Common/Exceptions.hpp"

namespace Sarwave::Core::MMWave::Config {

/// A .cfg line that could not be understood.
class ConfigFormatException : public Core::FormatException {
   public:
    explicit ConfigFormatException(const std::string& message)
        : Core::FormatException(message), m_message(message) {
    }

    /// The message is "Line {lineNumber}: {message}". The C# overload also takes the inner
    /// exception; here the reader throws this with std::throw_with_nested instead.
    ConfigFormatException(const std::string& message, int lineNumber)
        : ConfigFormatException("Line " + std::to_string(lineNumber) + ": " + message) {
        m_lineNumber = lineNumber;
    }

    /// The whole message (Exception.Message). Same as what(), except that what() stops at a NUL
    /// character, which a malformed argument quoted in the message may contain.
    [[nodiscard]] const std::string& message() const noexcept {
        return m_message;
    }

    /// 1-based line in the file, or 0 if the error wasn't tied to one.
    [[nodiscard]] int lineNumber() const noexcept {
        return m_lineNumber;
    }

   private:
    std::string m_message;
    int m_lineNumber = 0;
};

}  // namespace Sarwave::Core::MMWave::Config
