#include "MMWave/Configured/PeopleTracking/Tlv/TlvTypes.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Tlv {

std::string toString(TlvType type) {
    switch (type) {
        case TlvType::TlvTargetList:
            return "TlvTargetList";
        case TlvType::TlvTargetIndex:
            return "TlvTargetIndex";
        case TlvType::TlvTargetHeight:
            return "TlvTargetHeight";
        case TlvType::TlvPointCloud:
            return "TlvPointCloud";
        case TlvType::TlvPresenceIndication:
            return "TlvPresenceIndication";
    }
    return std::to_string(static_cast<std::uint32_t>(type));
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking::Tlv
