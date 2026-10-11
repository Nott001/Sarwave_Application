#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

/// `staticRACfarCfg`: Two-pass CFAR detection over the heatmap of static reflections. Only used
/// when StaticRangeAngleCfg enables static processing.
struct StaticRACfarCfg final : MMWave::Config::ConfigRecord<StaticRACfarCfg> {
    static constexpr std::string_view Name = "staticRACfarCfg";

    /// Always -1; subframes are not supported.
    int subFrameIdx;
    /// Range bins nearest the sensor excluded from detection.
    int cfarDiscardLeftRange;
    /// Range bins furthest from the sensor excluded from detection.
    int cfarDiscardRightRange;
    /// Angle bins excluded on the left edge.
    int cfarDiscardLeftAngle;
    /// Angle bins excluded on the right edge.
    int cfarDiscardRightAngle;
    /// Reference window size of the range CFAR, in bins.
    int refWinSizeRange;
    /// Reference window size of the angle CFAR, in bins.
    int refWinSizeAngle;
    /// Guard window size of the range CFAR, in bins.
    int guardWinSizeRange;
    /// Guard window size of the angle CFAR, in bins.
    int guardWinSizeAngle;
    /// Range CFAR threshold as a linear ratio over the estimated noise.
    float rangeThre;
    /// Angle CFAR threshold as a linear ratio over the estimated noise.
    float angleThre;
    /// Linear ratio to the strongest peak above which a local peak counts as a detection and not a
    /// sidelobe.
    float sidelobeThre;
    /// Whether range detections are confirmed by a second CFAR pass across angle.
    bool enable2ndPass;
    /// Unused; always 0.
    int dynamicFlag;

    StaticRACfarCfg(int subFrameIdx, int cfarDiscardLeftRange, int cfarDiscardRightRange,
                    int cfarDiscardLeftAngle, int cfarDiscardRightAngle, int refWinSizeRange,
                    int refWinSizeAngle, int guardWinSizeRange, int guardWinSizeAngle,
                    float rangeThre, float angleThre, float sidelobeThre, bool enable2ndPass,
                    int dynamicFlag)
        : subFrameIdx(subFrameIdx),
          cfarDiscardLeftRange(cfarDiscardLeftRange),
          cfarDiscardRightRange(cfarDiscardRightRange),
          cfarDiscardLeftAngle(cfarDiscardLeftAngle),
          cfarDiscardRightAngle(cfarDiscardRightAngle),
          refWinSizeRange(refWinSizeRange),
          refWinSizeAngle(refWinSizeAngle),
          guardWinSizeRange(guardWinSizeRange),
          guardWinSizeAngle(guardWinSizeAngle),
          rangeThre(rangeThre),
          angleThre(angleThre),
          sidelobeThre(sidelobeThre),
          enable2ndPass(enable2ndPass),
          dynamicFlag(dynamicFlag) {
    }

    static MMWave::Config::ConfigParameterPtr parse(MMWave::Config::ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override;

    bool operator==(const StaticRACfarCfg&) const = default;
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
