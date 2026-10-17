#pragma once

#include "Gui/Models/RadarData.hpp"
#include "Gui/ViewModels/Radar/RadarGridViewModel.hpp"
#include "Gui/ViewModels/StatusViewModel.hpp"
#include "Gui/ViewModels/ViewModelBase.hpp"

namespace Sarwave::Core::Gui::ViewModels {

/// The compact dashboard: status bar and radar grid.
class MinimalGuiViewModel : public ViewModelBase {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by GuiHost.")

    Q_PROPERTY(Sarwave::Core::Gui::ViewModels::StatusViewModel* status READ status CONSTANT)
    Q_PROPERTY(Sarwave::Core::Gui::ViewModels::Radar::RadarGridViewModel* radar READ radar CONSTANT)

   public:
    /// No data source; what the previewer shows.
    explicit MinimalGuiViewModel(DesignModeTag, QObject* parent = nullptr);

    explicit MinimalGuiViewModel(Models::RadarData& data, QObject* parent = nullptr);

    [[nodiscard]] StatusViewModel* status() const {
        return m_status;
    }
    [[nodiscard]] Radar::RadarGridViewModel* radar() const {
        return m_radar;
    }

   private:
    StatusViewModel* m_status;
    Radar::RadarGridViewModel* m_radar;
    // public MetricsViewModel Metrics { get; } = new();  (commented out in the C# version)

    LatestOnUiThread<StatusValues> m_statusPost{this, [this](StatusValues values) {
                                                    m_status->setPresenceDetected(values.presence);
                                                    m_status->setHoldSteady(values.holdSteady);
                                                }};
    RadarDataSubscription m_subscription;  // last: ended first
};

}  // namespace Sarwave::Core::Gui::ViewModels
