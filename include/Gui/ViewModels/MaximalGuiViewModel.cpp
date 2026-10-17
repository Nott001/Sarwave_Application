#include "Gui/ViewModels/MaximalGuiViewModel.hpp"

namespace Sarwave::Core::Gui::ViewModels {

MaximalGuiViewModel::MaximalGuiViewModel(DesignModeTag, QObject* parent)
    : ViewModelBase(parent),
      m_status(new StatusViewModel(this)),
      m_radar(new Radar::RadarGridViewModel(DesignMode, this)),
      m_objects(new Object::ObjectsPanelViewModel(DesignMode, this)) {
}

MaximalGuiViewModel::MaximalGuiViewModel(Models::RadarData& data, QObject* parent)
    : ViewModelBase(parent),
      m_status(new StatusViewModel(this)),
      m_radar(new Radar::RadarGridViewModel(data, this)),
      m_objects(new Object::ObjectsPanelViewModel(data, this)) {
    m_subscription = RadarDataSubscription(
        data, [this, &data] { m_statusPost.post({data.presence(), data.holdSteady()}); });
}

}  // namespace Sarwave::Core::Gui::ViewModels
