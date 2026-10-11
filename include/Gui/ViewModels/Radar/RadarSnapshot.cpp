#include "Gui/ViewModels/Radar/RadarSnapshot.hpp"

namespace Sarwave::Core::Gui::ViewModels::Radar {

const std::shared_ptr<const RadarSnapshot>& RadarSnapshot::empty() {
    static const std::shared_ptr<const RadarSnapshot> instance =
        std::make_shared<const RadarSnapshot>();
    return instance;
}

}  // namespace Sarwave::Core::Gui::ViewModels::Radar
