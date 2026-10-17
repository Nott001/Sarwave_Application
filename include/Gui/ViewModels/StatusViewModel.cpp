#include "Gui/ViewModels/StatusViewModel.hpp"

namespace Sarwave::Core::Gui::ViewModels {

void StatusViewModel::setPresenceDetected(bool value) {
    if (m_presenceDetected == value) return;
    m_presenceDetected = value;
    emit presenceDetectedChanged();
    emit statusTextChanged();
    emit badgeTextChanged();
}

void StatusViewModel::setHoldSteady(bool value) {
    if (m_holdSteady == value) return;
    m_holdSteady = value;
    emit holdSteadyChanged();
    emit statusTextChanged();
}

void StatusViewModel::setLastUpdated(const QString& value) {
    if (m_lastUpdated == value) return;
    m_lastUpdated = value;
    emit lastUpdatedChanged();
}

void StatusViewModel::setDeviceDescription(const QString& value) {
    if (m_deviceDescription == value) return;
    m_deviceDescription = value;
    emit deviceDescriptionChanged();
}

QString StatusViewModel::statusText() const {
    if (m_holdSteady) return QStringLiteral("HOLD STEADY");
    return m_presenceDetected ? QStringLiteral("TARGET ACQUIRED") : QStringLiteral("SCANNING…");
}

QString StatusViewModel::badgeText() const {
    return m_presenceDetected ? QStringLiteral("DETECTED") : QStringLiteral("CLEAR");
}

}  // namespace Sarwave::Core::Gui::ViewModels
