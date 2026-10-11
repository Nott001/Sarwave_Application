#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/ConfigEnum.hpp"
#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

/// `dynamicRangeAngleCfg`: How the range-angle heatmap of moving reflections is built (Capon
/// beamforming).
struct DynamicRangeAngleCfg final : MMWave::Config::ConfigRecord<DynamicRangeAngleCfg> {
    /// The selectable processing chains.
    enum class RangeAngleMethod : int {
        /// Range-azimuth detection, then elevation estimation. Suggested for wall mount.
        RangeAzimuthThenElevation = 1,
        /// Range-azimuth-elevation detection with angle zoom-in. Suggested for ceiling mount.
        RangeAzimuthElevationZoomIn = 2,
    };

    /// Doppler estimation methods.
    enum class DopplerMethod : int {
        /// Strongest peak of the Doppler spectrum; the only method supported.
        SinglePeakSearch = 0,
    };

    static constexpr std::string_view Name = "dynamicRangeAngleCfg";

    /// Always -1; subframes are not supported.
    int subFrameIdx;
    /// Angle search granularity in degrees: azimuth only for wall mount (not below 0.75), azimuth
    /// and elevation for ceiling mount (not below 7).
    float angleSearchStep;
    /// Diagonal loading factor for the spatial covariance matrix; 0.001 is TI's tested value.
    float rangeAngleDiagonalLoading;
    /// Which processing chain is used; this also decides which form `dynamic2DAngleCfg` takes.
    RangeAngleMethod rangeAngleEstMethod;
    /// How radial velocity is picked from the Doppler spectrum.
    DopplerMethod dopplerEstMethod;

    DynamicRangeAngleCfg(int subFrameIdx, float angleSearchStep, float rangeAngleDiagonalLoading,
                         RangeAngleMethod rangeAngleEstMethod, DopplerMethod dopplerEstMethod)
        : subFrameIdx(subFrameIdx),
          angleSearchStep(angleSearchStep),
          rangeAngleDiagonalLoading(rangeAngleDiagonalLoading),
          rangeAngleEstMethod(rangeAngleEstMethod),
          dopplerEstMethod(dopplerEstMethod) {
    }

    static MMWave::Config::ConfigParameterPtr parse(MMWave::Config::ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const DynamicRangeAngleCfg&) const = default;

    friend const MMWave::Config::EnumDefinition<RangeAngleMethod>& enumDefinition(RangeAngleMethod);

    friend const MMWave::Config::EnumDefinition<DopplerMethod>& enumDefinition(DopplerMethod);
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
