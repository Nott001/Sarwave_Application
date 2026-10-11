#include "MMWave/Configured/PeopleTracking/Config/IWR6843/Parameters/BoundaryBox.hpp"

#include <memory>

#include "MMWave/Config/ConfigArguments.hpp"
#include "MMWave/Config/ConfigText.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters {

MMWave::Config::ConfigParameterPtr BoundaryBox::parse(MMWave::Config::ConfigArguments& args) {
    // A braced initializer runs the reads left to right, in file order.
    return std::make_shared<const BoundaryBox>(
        BoundaryBox{args.floatValue("xMin"), args.floatValue("xMax"), args.floatValue("yMin"),
                    args.floatValue("yMax"), args.floatValue("zMin"), args.floatValue("zMax")});
}

std::string BoundaryBox::toString() const {
    return MMWave::Config::ConfigText::line(Name, xMin, xMax, yMin, yMax, zMin, zMax);
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Config::IWR6843::Parameters
