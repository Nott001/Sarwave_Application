#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "MMWave/Config/ConfigEnum.hpp"
#include "MMWave/Config/IConfigParameter.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

class Dynamic2DAngleElevationCfg;
class Dynamic2DAngleZoomInCfg;

/// `dynamic2DAngleCfg`: How the second angle of each detected point is estimated. The command has
/// two different argument lists depending on `dynamicRangeAngleCfg`'s `rangeAngleEstMethod`:
/// Dynamic2DAngleElevationCfg for method 1 (wall mount, nine arguments) and
/// Dynamic2DAngleZoomInCfg for method 2 (ceiling mount, seven arguments).
///
/// The two forms compare with operator== as their own types; there is no comparison through this
/// base.
class Dynamic2DAngleCfg : public MMWave::Config::ConfigRecord<Dynamic2DAngleCfg> {
   public:
    static constexpr std::string_view Name = "dynamic2DAngleCfg";

    /// Always -1; subframes are not supported.
    int subFrameIdx;

    /// Picks the form by argument count, since the line itself doesn't say which method it is
    /// for.
    static MMWave::Config::ConfigParameterPtr parse(MMWave::Config::ConfigArguments& args);

    [[nodiscard]] std::string paramName() const override {
        return std::string(Name);
    }

    [[nodiscard]] std::string toString() const override = 0;

   protected:
    explicit Dynamic2DAngleCfg(int subFrameIdx) : subFrameIdx(subFrameIdx) {
    }

   private:
    friend class Dynamic2DAngleElevationCfg;
    friend class Dynamic2DAngleZoomInCfg;

    bool operator==(const Dynamic2DAngleCfg&) const = default;
};

/// `dynamic2DAngleCfg` for `rangeAngleEstMethod` 1 (wall mount): after range-azimuth detection,
/// the elevation of each point is taken from the strongest peak of a 1D elevation spectrum.
class Dynamic2DAngleElevationCfg final : public Dynamic2DAngleCfg {
   public:
    static constexpr int ArgumentCount = 9;

    /// Elevation search granularity in degrees; not below 1.
    float elevSearchStep;
    /// Diagonal loading factor for the spatial covariance matrix; 0.03 is TI's tested value.
    float angleDiagonalLoading;
    /// Peaks to search for in elevation; only 1 is supported.
    int maxNpeak2Search;
    /// Samples either side of the peak to include; not supported in this method, always 0.
    int peakExpSamples;
    /// True = elevation estimation only; the only supported value.
    bool elevOnly;
    /// Reserved for a future multi-peak search.
    float sidelobeThre;
    /// Reserved for a future peak expansion.
    float peakExpRelThre;
    /// Reserved for a future peak expansion.
    float peakExpSnrThre;

    /// `subFrameIdx`: always -1; subframes are not supported.
    Dynamic2DAngleElevationCfg(int subFrameIdx, float elevSearchStep, float angleDiagonalLoading,
                               int maxNpeak2Search, int peakExpSamples, bool elevOnly,
                               float sidelobeThre, float peakExpRelThre, float peakExpSnrThre)
        : Dynamic2DAngleCfg(subFrameIdx),
          elevSearchStep(elevSearchStep),
          angleDiagonalLoading(angleDiagonalLoading),
          maxNpeak2Search(maxNpeak2Search),
          peakExpSamples(peakExpSamples),
          elevOnly(elevOnly),
          sidelobeThre(sidelobeThre),
          peakExpRelThre(peakExpRelThre),
          peakExpSnrThre(peakExpSnrThre) {
    }

    static std::shared_ptr<const Dynamic2DAngleElevationCfg> parseArguments(
        MMWave::Config::ConfigArguments& args);

    [[nodiscard]] std::string toString() const override;

    bool operator==(const Dynamic2DAngleElevationCfg&) const = default;
};

/// `dynamic2DAngleCfg` for `rangeAngleEstMethod` 2 (ceiling mount): each coarse azimuth-elevation
/// detection is zoomed into on a finer grid.
class Dynamic2DAngleZoomInCfg final : public Dynamic2DAngleCfg {
   public:
    /// What each `localMaxCheck` number means.
    enum class LocalMax : int {
        /// Every coarse detection is zoomed into, sidelobes included.
        None = 0,
        /// Only coarse peaks that are a local maximum in elevation.
        Elevation = 1,
        /// Only coarse peaks that are a local maximum in both elevation and azimuth.
        ElevationAndAzimuth = 2,
    };

    static constexpr int ArgumentCount = 7;

    /// Fine step = `angleSearchStep` divided by this; not above 6.
    int zoominFactor;
    /// Coarse neighbouring bins covered by the zoom-in; only 1 is supported.
    int zoominNn8bors;
    /// Samples either side of a fine peak that may also be declared points.
    int peakExpSamples;
    /// Starting relative threshold for including a peak's neighbours; 1 lets the algorithm
    /// derive it.
    float peakExpRelThre;
    /// Linear SNR a coarse peak needs before it may be expanded.
    float peakExpSnrThre;
    /// Which coarse detections are allowed to be zoomed into.
    LocalMax localMaxCheck;

    /// `subFrameIdx`: always -1; subframes are not supported.
    Dynamic2DAngleZoomInCfg(int subFrameIdx, int zoominFactor, int zoominNn8bors,
                            int peakExpSamples, float peakExpRelThre, float peakExpSnrThre,
                            LocalMax localMaxCheck)
        : Dynamic2DAngleCfg(subFrameIdx),
          zoominFactor(zoominFactor),
          zoominNn8bors(zoominNn8bors),
          peakExpSamples(peakExpSamples),
          peakExpRelThre(peakExpRelThre),
          peakExpSnrThre(peakExpSnrThre),
          localMaxCheck(localMaxCheck) {
    }

    static std::shared_ptr<const Dynamic2DAngleZoomInCfg> parseArguments(
        MMWave::Config::ConfigArguments& args);

    [[nodiscard]] std::string toString() const override;

    bool operator==(const Dynamic2DAngleZoomInCfg&) const = default;

    friend const MMWave::Config::EnumDefinition<LocalMax>& enumDefinition(LocalMax);
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
