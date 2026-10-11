#pragma once

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace Sarwave::Core::MMWave::Config {

/// One defined value of an enum and its C# name.
template <class TEnum>
struct EnumMember {
    TEnum value;
    std::string_view name;
};

/// What C# finds out about an enum by reflection and ConfigArguments::enumValue needs: its
/// defined values with their names, and whether it is a [Flags] enum.
///
/// Each enum used in a .cfg command provides one through a function found by argument-dependent
/// lookup, normally a friend of the class the enum is nested in:
///
///     friend const Config::EnumDefinition<Mode>& enumDefinition(Mode);
template <class TEnum>
class EnumDefinition {
   public:
    EnumDefinition(bool isFlags, std::initializer_list<EnumMember<TEnum>> members)
        : m_isFlags(isFlags), m_members(members) {
        // Enum.GetValues order: by the value's unsigned magnitude, so -1 comes last.
        std::ranges::stable_sort(m_members, {}, [](const EnumMember<TEnum>& member) {
            return static_cast<std::make_unsigned_t<std::underlying_type_t<TEnum>>>(
                std::to_underlying(member.value));
        });
    }

    /// Whether the enum is a bit mask ([Flags]), so any combination of its values is valid.
    [[nodiscard]] bool isFlags() const {
        return m_isFlags;
    }

    /// Every defined value, in the order Enum.GetValues lists them.
    [[nodiscard]] std::span<const EnumMember<TEnum>> members() const {
        return m_members;
    }

    /// Enum.IsDefined: whether `value` is one of the named values.
    [[nodiscard]] bool isDefined(TEnum value) const {
        return std::ranges::any_of(
            m_members, [value](const EnumMember<TEnum>& member) { return member.value == value; });
    }

   private:
    bool m_isFlags;
    std::vector<EnumMember<TEnum>> m_members;
};

/// An enum that ConfigArguments can read: one with an enumDefinition() (see EnumDefinition).
template <class TEnum>
concept ConfigEnum = std::is_enum_v<TEnum> && requires(TEnum value) {
    { enumDefinition(value) } -> std::same_as<const EnumDefinition<TEnum>&>;
};

}  // namespace Sarwave::Core::MMWave::Config
