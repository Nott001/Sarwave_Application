#include "Gui/ViewModels/MinimalGuiViewModel.hpp"

namespace Sarwave::Core::Gui::ViewModels {

MinimalGuiViewModel::MinimalGuiViewModel(DesignModeTag, QObject* parent)
    : ViewModelBase(parent),
      m_status(new StatusViewModel(this)),
      m_radar(new Radar::RadarGridViewModel(DesignMode, this)) {
}

MinimalGuiViewModel::MinimalGuiViewModel(Models::RadarData& data, QObject* parent)
    : ViewModelBase(parent),
      m_status(new StatusViewModel(this)),
      m_radar(new Radar::RadarGridViewModel(data, this)) {
    m_subscription = RadarDataSubscription(data, [this, &data] {
        const bool presence = data.presence();
        onUiThread([this, presence] { m_status->setPresenceDetected(presence); });
    });
}

}  // namespace Sarwave::Core::Gui::ViewModels
