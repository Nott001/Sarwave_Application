#pragma once

#include <concepts>
#include <memory>
#include <string>
#include <string_view>

namespace Sarwave::Core::MMWave::Config {

class ConfigArguments;

/// One line of a .cfg file: a CLI command and its arguments (or a Comment).
class IConfigParameter {
   public:
    virtual ~IConfigParameter() = default;

    /// The command word that starts the line, e.g. "profileCfg".
    [[nodiscard]] virtual std::string paramName() const = 0;

    /// The whole line exactly as it should be written to a .cfg file or sent to the device.
    [[nodiscard]] virtual std::string toString() const = 0;

   protected:
    IConfigParameter() = default;
    IConfigParameter(const IConfigParameter&) = default;
    IConfigParameter(IConfigParameter&&) = default;
    IConfigParameter& operator=(const IConfigParameter&) = default;
    IConfigParameter& operator=(IConfigParameter&&) = default;
};

/// How a line is held: immutable and shared, like the C# record references, so one instance
/// (Comment::blank(), an AopBoard row) can sit in any number of layouts.
using ConfigParameterPtr = std::shared_ptr<const IConfigParameter>;

/// The base of a concrete parameter type `Self`. It holds nothing; it exists so that `Self` can
/// `= default` its operator== and get the member-wise value equality a C# record has. Its own
/// operator== is private to `Self`, so lines can only be compared as their concrete types.
template <class Self>
class ConfigRecord : public IConfigParameter {
    friend Self;

    bool operator==(const ConfigRecord&) const {
        return true;
    }
};

/// IParsableConfigParameter: a parameter type that knows its own command word and how to build
/// itself from the arguments that follow it. This is what lets a ConfigParameterRegistry pick the
/// right type for a line.
///
/// - `T::Name`: the command word; the same value every instance returns from paramName().
/// - `T::parse(args)`: reads this command's arguments in order. Leftover arguments are rejected
///   by the caller. Throws ConfigFormatException when an argument is missing or is not a valid
///   number.
template <class T>
concept ParsableConfigParameter =
    std::derived_from<T, IConfigParameter> && requires(ConfigArguments& args) {
        { T::Name } -> std::convertible_to<std::string_view>;
        { T::parse(args) } -> std::same_as<ConfigParameterPtr>;
    };

}  // namespace Sarwave::Core::MMWave::Config
