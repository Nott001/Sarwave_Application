#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Config {

/// A whole .cfg file as an ordered list of its lines. The order is the order the commands are
/// sent to the device, which matters (sensorStop first, sensorStart last), so it is kept exactly.
/// A concrete layout says which commands it understands and offers them by name.
class ConfigLayout {
   public:
    virtual ~ConfigLayout() = default;

    /// Every line in file order, comments and blank lines included.
    [[nodiscard]] const std::vector<ConfigParameterPtr>& parameters() const {
        return m_parameters;
    }

    /// The file's text: each line's toString() in order, one per line.
    [[nodiscard]] std::string toString() const {
        return toString("\n");
    }

    /// `newLine`: line ending to use, e.g. "\r\n" to match the .cfg files TI ships.
    [[nodiscard]] std::string toString(std::string_view newLine) const;

   protected:
    /// `parameters` must not hold null pointers.
    explicit ConfigLayout(std::vector<ConfigParameterPtr> parameters);

    ConfigLayout(const ConfigLayout&) = default;
    ConfigLayout(ConfigLayout&&) = default;
    ConfigLayout& operator=(const ConfigLayout&) = default;
    ConfigLayout& operator=(ConfigLayout&&) = default;

    // A line is of type T if it is a T or derives from one, so a command with several forms
    // (dynamic2DAngleCfg) can be asked for by the type they share. The C# version files the
    // lines by type up front; a layout is a few dozen lines, so these simply scan them.

    /// The first line of type `T`, or null if the file has none. Points into this layout.
    template <class T>
    [[nodiscard]] const T* find() const {
        for (const ConfigParameterPtr& parameter : m_parameters) {
            if (const auto* typed = dynamic_cast<const T*>(parameter.get())) return typed;
        }

        return nullptr;
    }

    /// Every line of type `T`, in file order. Points into this layout.
    template <class T>
    [[nodiscard]] std::vector<const T*> all() const {
        std::vector<const T*> found;
        for (const ConfigParameterPtr& parameter : m_parameters) {
            if (const auto* typed = dynamic_cast<const T*>(parameter.get())) {
                found.push_back(typed);
            }
        }

        return found;
    }

   private:
    std::vector<ConfigParameterPtr> m_parameters;
};

}  // namespace Sarwave::Core::MMWave::Config
