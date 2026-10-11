#pragma once

#include <string>
#include <utility>
#include <vector>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Config {

/// A command no parameter type is registered for, kept exactly as its words were written so a
/// config containing it can still be read, inspected and written back.
class RawParameter final : public ConfigRecord<RawParameter> {
   public:
    RawParameter(std::string paramName, std::vector<std::string> arguments)
        : m_paramName(std::move(paramName)), m_arguments(std::move(arguments)) {
    }

    [[nodiscard]] std::string paramName() const override {
        return m_paramName;
    }

    [[nodiscard]] const std::vector<std::string>& arguments() const {
        return m_arguments;
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const RawParameter&) const = default;

   private:
    std::string m_paramName;
    std::vector<std::string> m_arguments;
};

}  // namespace Sarwave::Core::MMWave::Config
