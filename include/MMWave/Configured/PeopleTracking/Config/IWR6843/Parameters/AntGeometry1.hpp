#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

/// `antGeometry1`: Position index of each virtual antenna along the elevation axis (0, -1, -2,
/// ...), in virtual antenna order. Board specific: ISK, ODS and AOP each have their own row.
struct AntGeometry1 final : MMWave::Config::ConfigRecord<AntGeometry1> {
    static constexpr std::string_view Name = "antGeometry1";

    /// One value per virtual antenna (12 for a 3 TX, 4 RX device).
    std::vector<int> elevationIndexes;

    explicit AntGeometry1(std::vector<int> elevationIndexes)
        : elevationIndexes(std::move(elevationIndexes)) {
    }

    static MMWave::Config::ConfigParameterPtr parse(MMWave::Config::ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const AntGeometry1&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
