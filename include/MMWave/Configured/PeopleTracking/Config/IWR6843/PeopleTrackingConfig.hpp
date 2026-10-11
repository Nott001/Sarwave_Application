#pragma once

#include <utility>
#include <vector>

#include "MMWave/Config/ConfigLayout.hpp"
#include "MMWave/Config/Parameters/AdcCfg.hpp"
#include "MMWave/Config/Parameters/AdcbufCfg.hpp"
#include "MMWave/Config/Parameters/BpmCfg.hpp"
#include "MMWave/Config/Parameters/ChannelCfg.hpp"
#include "MMWave/Config/Parameters/ChirpCfg.hpp"
#include "MMWave/Config/Parameters/CompRangeBiasAndRxChanPhase.hpp"
#include "MMWave/Config/Parameters/DfeDataOutputMode.hpp"
#include "MMWave/Config/Parameters/FrameCfg.hpp"
#include "MMWave/Config/Parameters/LowPower.hpp"
#include "MMWave/Config/Parameters/ProfileCfg.hpp"
#include "MMWave/Config/RawParameter.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/AllocationParam.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/AntGeometry0.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/AntGeometry1.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/AntPhaseRot.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/BoundaryBox.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/Dynamic2DAngleCfg.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/DynamicRACfarCfg.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/DynamicRangeAngleCfg.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/FineMotionCfg.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/FovCfg.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/GatingParam.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/MaxAcceleration.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/PresenceBoundaryBox.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/SensorPosition.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/StateParam.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/StaticBoundaryBox.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/StaticRACfarCfg.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/StaticRangeAngleCfg.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/TrackingCfg.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843 {

/// A 3D people tracking .cfg file. parameters() holds every line in file order (comments
/// included) and is what toString() writes back; the accessors below are shortcuts to the
/// individual commands within it.
///
/// An accessor returns null when the file doesn't contain that command. TI's guides call every
/// command here mandatory except FineMotionCfg and BpmCfg, but this type only reports what is in
/// the file; it does not enforce that. The pointers point into this config.
class PeopleTrackingConfig final : public MMWave::Config::ConfigLayout {
   public:
    explicit PeopleTrackingConfig(std::vector<MMWave::Config::ConfigParameterPtr> parameters)
        : ConfigLayout(std::move(parameters)) {
    }

    // ---- sensor front end (shared with every SDK demo) -------------------------------------
    [[nodiscard]] const MMWave::Config::Parameters::DfeDataOutputMode* dfeDataOutputMode() const {
        return find<MMWave::Config::Parameters::DfeDataOutputMode>();
    }

    [[nodiscard]] const MMWave::Config::Parameters::ChannelCfg* channelCfg() const {
        return find<MMWave::Config::Parameters::ChannelCfg>();
    }

    [[nodiscard]] const MMWave::Config::Parameters::AdcCfg* adcCfg() const {
        return find<MMWave::Config::Parameters::AdcCfg>();
    }

    [[nodiscard]] const MMWave::Config::Parameters::AdcbufCfg* adcbufCfg() const {
        return find<MMWave::Config::Parameters::AdcbufCfg>();
    }

    [[nodiscard]] const MMWave::Config::Parameters::LowPower* lowPower() const {
        return find<MMWave::Config::Parameters::LowPower>();
    }

    [[nodiscard]] const MMWave::Config::Parameters::ProfileCfg* profileCfg() const {
        return find<MMWave::Config::Parameters::ProfileCfg>();
    }

    /// One line per distinct chirp, normally three (one per transmitter).
    [[nodiscard]] std::vector<const MMWave::Config::Parameters::ChirpCfg*> chirpCfgs() const {
        return all<MMWave::Config::Parameters::ChirpCfg>();
    }

    [[nodiscard]] const MMWave::Config::Parameters::FrameCfg* frameCfg() const {
        return find<MMWave::Config::Parameters::FrameCfg>();
    }

    /// Optional; BPM is disabled when absent.
    [[nodiscard]] const MMWave::Config::Parameters::BpmCfg* bpmCfg() const {
        return find<MMWave::Config::Parameters::BpmCfg>();
    }

    // ---- detection layer -------------------------------------------------------------------
    [[nodiscard]] const Parameters::DynamicRangeAngleCfg* dynamicRangeAngleCfg() const {
        return find<Parameters::DynamicRangeAngleCfg>();
    }

    [[nodiscard]] const Parameters::DynamicRACfarCfg* dynamicRACfarCfg() const {
        return find<Parameters::DynamicRACfarCfg>();
    }

    /// Either form: Dynamic2DAngleElevationCfg or Dynamic2DAngleZoomInCfg.
    [[nodiscard]] const Parameters::Dynamic2DAngleCfg* dynamic2DAngleCfg() const {
        return find<Parameters::Dynamic2DAngleCfg>();
    }

    [[nodiscard]] const Parameters::StaticRangeAngleCfg* staticRangeAngleCfg() const {
        return find<Parameters::StaticRangeAngleCfg>();
    }

    [[nodiscard]] const Parameters::StaticRACfarCfg* staticRACfarCfg() const {
        return find<Parameters::StaticRACfarCfg>();
    }

    /// Optional; fine motion mode is disabled when absent.
    [[nodiscard]] const Parameters::FineMotionCfg* fineMotionCfg() const {
        return find<Parameters::FineMotionCfg>();
    }

    // ---- board -----------------------------------------------------------------------------
    [[nodiscard]] const Parameters::AntGeometry0* antGeometry0() const {
        return find<Parameters::AntGeometry0>();
    }

    [[nodiscard]] const Parameters::AntGeometry1* antGeometry1() const {
        return find<Parameters::AntGeometry1>();
    }

    [[nodiscard]] const Parameters::AntPhaseRot* antPhaseRot() const {
        return find<Parameters::AntPhaseRot>();
    }

    [[nodiscard]] const Parameters::FovCfg* fovCfg() const {
        return find<Parameters::FovCfg>();
    }

    [[nodiscard]] const MMWave::Config::Parameters::CompRangeBiasAndRxChanPhase*
    compRangeBiasAndRxChanPhase() const {
        return find<MMWave::Config::Parameters::CompRangeBiasAndRxChanPhase>();
    }

    // ---- tracker layer ---------------------------------------------------------------------
    [[nodiscard]] const Parameters::StaticBoundaryBox* staticBoundaryBox() const {
        return find<Parameters::StaticBoundaryBox>();
    }

    [[nodiscard]] const Parameters::BoundaryBox* boundaryBox() const {
        return find<Parameters::BoundaryBox>();
    }

    [[nodiscard]] const Parameters::PresenceBoundaryBox* presenceBoundaryBox() const {
        return find<Parameters::PresenceBoundaryBox>();
    }

    [[nodiscard]] const Parameters::SensorPosition* sensorPosition() const {
        return find<Parameters::SensorPosition>();
    }

    [[nodiscard]] const Parameters::GatingParam* gatingParam() const {
        return find<Parameters::GatingParam>();
    }

    [[nodiscard]] const Parameters::StateParam* stateParam() const {
        return find<Parameters::StateParam>();
    }

    [[nodiscard]] const Parameters::AllocationParam* allocationParam() const {
        return find<Parameters::AllocationParam>();
    }

    [[nodiscard]] const Parameters::MaxAcceleration* maxAcceleration() const {
        return find<Parameters::MaxAcceleration>();
    }

    [[nodiscard]] const Parameters::TrackingCfg* trackingCfg() const {
        return find<Parameters::TrackingCfg>();
    }

    /// Commands in the file that are not part of the people tracking demo (or are misspelt).
    /// They are kept and written back unchanged.
    [[nodiscard]] std::vector<const MMWave::Config::RawParameter*> unrecognised() const {
        return all<MMWave::Config::RawParameter>();
    }
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843
