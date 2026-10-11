#include "MMWave/Configured/PeopleTracking/Config/IWR6843/AopBoard.hpp"

#include <vector>

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843 {

const std::shared_ptr<const Parameters::AntGeometry0>& AopBoard::antGeometry0() {
    static const auto instance = std::make_shared<const Parameters::AntGeometry0>(
        std::vector<int>{-1, -1, 0, 0, -3, -3, -2, -2, -1, -1, 0, 0});
    return instance;
}

const std::shared_ptr<const Parameters::AntGeometry1>& AopBoard::antGeometry1() {
    static const auto instance = std::make_shared<const Parameters::AntGeometry1>(
        std::vector<int>{-1, 0, -1, 0, -3, -2, -3, -2, -3, -2, -3, -2});
    return instance;
}

const std::shared_ptr<const Parameters::AntPhaseRot>& AopBoard::antPhaseRot() {
    using enum Parameters::AntPhaseRot::Rotation;
    static const auto instance = std::make_shared<const Parameters::AntPhaseRot>(
        std::vector<Parameters::AntPhaseRot::Rotation>{
            None,
            Inverted,
            None,
            Inverted,
            None,
            Inverted,
            None,
            Inverted,
            None,
            Inverted,
            None,
            Inverted,
        });
    return instance;
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843
