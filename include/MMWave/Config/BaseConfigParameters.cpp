#include "MMWave/Config/BaseConfigParameters.hpp"

#include "MMWave/Config/Parameters/AdcCfg.hpp"
#include "MMWave/Config/Parameters/AdcbufCfg.hpp"
#include "MMWave/Config/Parameters/BpmCfg.hpp"
#include "MMWave/Config/Parameters/ChannelCfg.hpp"
#include "MMWave/Config/Parameters/ChirpCfg.hpp"
#include "MMWave/Config/Parameters/CompRangeBiasAndRxChanPhase.hpp"
#include "MMWave/Config/Parameters/DfeDataOutputMode.hpp"
#include "MMWave/Config/Parameters/FlushCfg.hpp"
#include "MMWave/Config/Parameters/FrameCfg.hpp"
#include "MMWave/Config/Parameters/LowPower.hpp"
#include "MMWave/Config/Parameters/ProfileCfg.hpp"
#include "MMWave/Config/Parameters/SensorStart.hpp"
#include "MMWave/Config/Parameters/SensorStop.hpp"

namespace Sarwave::Core::MMWave::Config {

ConfigParameterRegistry& BaseConfigParameters::addBaseParameters(
    ConfigParameterRegistry& registry) {
    using namespace Parameters;
    return registry.add<SensorStop>()
        .add<FlushCfg>()
        .add<SensorStart>()
        .add<DfeDataOutputMode>()
        .add<ChannelCfg>()
        .add<AdcCfg>()
        .add<AdcbufCfg>()
        .add<LowPower>()
        .add<ProfileCfg>()
        .add<ChirpCfg>()
        .add<FrameCfg>()
        .add<BpmCfg>()
        .add<CompRangeBiasAndRxChanPhase>();
}

}  // namespace Sarwave::Core::MMWave::Config
