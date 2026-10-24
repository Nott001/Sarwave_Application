#pragma once

#include <QString>
#include <optional>

#include "Gui/ViewModels/ViewModelBase.hpp"
#include "Imu/Gating/MotionGate.hpp"

namespace Sarwave::Core::Gui::ViewModels {

/// The IMU strip under the desktop dashboard's top bar (wireless and recorded apps, not the
/// device's own screen): the tilt and motion the gate judged the latest frame on, and which IMU
/// axis points up, for checking the mounting (docs/imu_mounting_checks.md) from a laptop.
///
/// Every value changes with each frame, so they share one notify signal.
class ImuStatusViewModel : public ViewModelBase {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by MaximalGuiViewModel.")

    Q_PROPERTY(bool isAvailable READ isAvailable NOTIFY shownChanged)
    Q_PROPERTY(QString pitch READ pitch NOTIFY shownChanged)
    Q_PROPERTY(QString roll READ roll NOTIFY shownChanged)
    Q_PROPERTY(QString upAxis READ upAxis NOTIFY shownChanged)
    Q_PROPERTY(QString turning READ turning NOTIFY shownChanged)
    Q_PROPERTY(QString gate READ gate NOTIFY shownChanged)
    Q_PROPERTY(bool isSteady READ isSteady NOTIFY shownChanged)
    Q_PROPERTY(bool isMoving READ isMoving NOTIFY shownChanged)

   public:
    using ViewModelBase::ViewModelBase;

    /// False until a frame comes with IMU data, and while the app runs without the IMU.
    [[nodiscard]] bool isAvailable() const {
        return m_isAvailable;
    }
    [[nodiscard]] QString pitch() const {
        return m_pitch;
    }
    [[nodiscard]] QString roll() const {
        return m_roll;
    }
    [[nodiscard]] QString upAxis() const {
        return m_upAxis;
    }
    [[nodiscard]] QString turning() const {
        return m_turning;
    }
    [[nodiscard]] QString gate() const {
        return m_gate;
    }
    [[nodiscard]] bool isSteady() const {
        return m_isSteady;
    }
    [[nodiscard]] bool isMoving() const {
        return m_isMoving;
    }

    /// Shows the IMU's verdict on the latest frame; nullopt when the app has no IMU.
    void show(const std::optional<Imu::Gating::ImuFrameState>& state);

   signals:
    void shownChanged();

   private:
    bool m_isAvailable = false;
    QString m_pitch = QStringLiteral("-");
    QString m_roll = QStringLiteral("-");
    QString m_upAxis = QStringLiteral("-");
    QString m_turning = QStringLiteral("-");
    QString m_gate = QStringLiteral("NO DATA");
    bool m_isSteady = false;
    bool m_isMoving = false;
};

}  // namespace Sarwave::Core::Gui::ViewModels
