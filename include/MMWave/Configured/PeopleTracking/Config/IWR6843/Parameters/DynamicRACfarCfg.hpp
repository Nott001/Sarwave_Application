#pragma once

#include <string>
#include <string_view>

#include "MMWave/Config/ConfigEnum.hpp"
#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

/// `dynamicRACfarCfg`: Two-pass CFAR detection over the heatmap of moving reflections.
struct DynamicRACfarCfg final : MMWave::Config::ConfigRecord<DynamicRACfarCfg> {
    /// What each `dynamicFlag` number means. The guide gives both 0 and 1 the meaning "disabled".
    enum class DynamicMode : int {
        /// Fixed thresholds.
        Disabled = 0,
        /// Also fixed thresholds; this is the value the shipped wall-mount configs use.
        DisabledAlt = 1,
        /// Threshold follows the expected signal strength over range.
        Range = 2,
        /// Threshold follows the expected signal strength over range and angle.
        RangeAndAngle = 3,
    };

    static constexpr std::string_view Name = "dynamicRACfarCfg";

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
    /// Whether the CFAR thresholds follow the expected signal strength.
    DynamicMode dynamicFlag;

    DynamicRACfarCfg(int subFrameIdx, int cfarDiscardLeftRange, int cfarDiscardRightRange,
                     int cfarDiscardLeftAngle, int cfarDiscardRightAngle, int refWinSizeRange,
                     int refWinSizeAngle, int guardWinSizeRange, int guardWinSizeAngle,
                     float rangeThre, float angleThre, float sidelobeThre, bool enable2ndPass,
                     DynamicMode dynamicFlag)
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

    bool operator==(const DynamicRACfarCfg&) const = default;

    friend const MMWave::Config::EnumDefinition<DynamicMode>& enumDefinition(DynamicMode);
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
