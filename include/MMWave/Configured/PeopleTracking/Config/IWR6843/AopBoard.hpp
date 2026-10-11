#pragma once

#include <memory>

#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/AntGeometry0.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/AntGeometry1.hpp"
#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/AntPhaseRot.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843 {

/// The values that are fixed by the IWR6843AOP board itself: where its twelve virtual antennas
/// sit and which of them are fed from the opposite side. They come from the "AOP" rows of the
/// detection layer tuning guide (section 3.3) and are the same in every AOP config, whatever
/// the range or tuning. The ISK and ODS boards use the same commands with different values.
struct AopBoard {
    /// `antGeometry0`: azimuth position index of each virtual antenna.
    static const std::shared_ptr<const Parameters::AntGeometry0>& antGeometry0();

    /// `antGeometry1`: elevation position index of each virtual antenna.
    static const std::shared_ptr<const Parameters::AntGeometry1>& antGeometry1();

    /// `antPhaseRot`: every second virtual antenna is fed from the opposite side.
    static const std::shared_ptr<const Parameters::AntPhaseRot>& antPhaseRot();
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843
