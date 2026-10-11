#pragma once

#include "Gui/Models/RadarData.hpp"
#include "Gui/ViewModels/Object/ObjectsPanelViewModel.hpp"
#include "Gui/ViewModels/Radar/RadarGridViewModel.hpp"
#include "Gui/ViewModels/StatusViewModel.hpp"
#include "Gui/ViewModels/ViewModelBase.hpp"

namespace Sarwave::Core::Gui::ViewModels {

/// The full dashboard: status bar, radar grid and the objects panel.
class MaximalGuiViewModel : public ViewModelBase {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by GuiHost.")

    Q_PROPERTY(Sarwave::Core::Gui::ViewModels::StatusViewModel* status READ status CONSTANT)
    Q_PROPERTY(Sarwave::Core::Gui::ViewModels::Radar::RadarGridViewModel* radar READ radar CONSTANT)
    Q_PROPERTY(Sarwave::Core::Gui::ViewModels::Object::ObjectsPanelViewModel* objects READ objects
                   CONSTANT)

   public:
    /// No data source; what the previewer shows.
    explicit MaximalGuiViewModel(DesignModeTag, QObject* parent = nullptr);

    explicit MaximalGuiViewModel(Models::RadarData& data, QObject* parent = nullptr);

    [[nodiscard]] StatusViewModel* status() const {
        return m_status;
    }
    [[nodiscard]] Radar::RadarGridViewModel* radar() const {
        return m_radar;
    }
    [[nodiscard]] Object::ObjectsPanelViewModel* objects() const {
        return m_objects;
    }

   private:
    StatusViewModel* m_status;
    Radar::RadarGridViewModel* m_radar;
    Object::ObjectsPanelViewModel* m_objects;

    RadarDataSubscription m_subscription;  // last: ended first
};

}  // namespace Sarwave::Core::Gui::ViewModels
