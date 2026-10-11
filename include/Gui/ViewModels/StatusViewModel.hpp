#pragma once

#include <QString>

#include "Gui/ViewModels/ViewModelBase.hpp"

namespace Sarwave::Core::Gui::ViewModels {

/// Values shown in the top bar.
class StatusViewModel : public ViewModelBase {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by the dashboard view models.")

    Q_PROPERTY(bool presenceDetected READ presenceDetected WRITE setPresenceDetected NOTIFY
                   presenceDetectedChanged)
    Q_PROPERTY(QString lastUpdated READ lastUpdated WRITE setLastUpdated NOTIFY lastUpdatedChanged)
    Q_PROPERTY(QString deviceDescription READ deviceDescription WRITE setDeviceDescription NOTIFY
                   deviceDescriptionChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString badgeText READ badgeText NOTIFY badgeTextChanged)

   public:
    using ViewModelBase::ViewModelBase;

    [[nodiscard]] bool presenceDetected() const {
        return m_presenceDetected;
    }
    void setPresenceDetected(bool value);

    [[nodiscard]] QString lastUpdated() const {
        return m_lastUpdated;
    }
    void setLastUpdated(const QString& value);

    [[nodiscard]] QString deviceDescription() const {
        return m_deviceDescription;
    }
    void setDeviceDescription(const QString& value);

    [[nodiscard]] QString statusText() const;
    [[nodiscard]] QString badgeText() const;

   signals:
    void presenceDetectedChanged();
    void lastUpdatedChanged();
    void deviceDescriptionChanged();
    void statusTextChanged();
    void badgeTextChanged();

   private:
    bool m_presenceDetected = false;
    QString m_lastUpdated;
    QString m_deviceDescription = QStringLiteral("IWR6843AOP · mmWave Radar");
};

}  // namespace Sarwave::Core::Gui::ViewModels
