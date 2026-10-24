#include "Gui/ViewModels/ImuStatusViewModel.hpp"

#include <cmath>
#include <cstdlib>

#include "Integration/PeopleTracking/ImuFrames.hpp"
#include "MMWave/Common/DotNetFormat.hpp"

namespace Sarwave::Core::Gui::ViewModels {

using Imu::Gating::MotionGateState;
using MMWave::Vector3;

namespace {

/// (MathF.Round(value, 1) + 0f).ToString("+0.0;-0.0"): always signed; + 0 turns −0 into 0, so
/// zero prints as +0.0.
QString signedText(float value) {
    const float rounded = std::nearbyint(value * 10.0f) / 10.0f + 0.0f;
    const std::string digits = MMWave::DotNet::formatFixed(std::abs(rounded), 1, 1);
    return QString::fromStdString((rounded < 0 ? "-" : "+") + digits);
}

QString nearestAxis(Vector3 v) {
    const float ax = std::abs(v.x);
    const float ay = std::abs(v.y);
    const float az = std::abs(v.z);
    float value = v.z;
    char axis = 'Z';
    if (ax >= ay && ax >= az) {
        value = v.x;
        axis = 'X';
    } else if (ay >= az) {
        value = v.y;
        axis = 'Y';
    }
    return QString(value >= 0 ? '+' : '-') + QChar(axis);
}

}  // namespace

void ImuStatusViewModel::show(const std::optional<Imu::Gating::ImuFrameState>& state) {
    m_isAvailable = state.has_value();
    if (!state) {
        emit shownChanged();
        return;
    }
    const auto& s = *state;

    switch (s.state) {
        case MotionGateState::Steady:
            m_gate = QStringLiteral("STEADY");
            break;
        case MotionGateState::Settling:
            m_gate = QStringLiteral("SETTLING");
            break;
        case MotionGateState::Moving:
            m_gate = QStringLiteral("MOVING");
            break;
        default:
            m_gate = QStringLiteral("NO DATA");
            break;
    }
    m_isSteady = s.state == MotionGateState::Steady;
    m_isMoving = s.state == MotionGateState::Moving;

    if (s.state == MotionGateState::Unavailable || !s.tilt.isValid()) {
        m_pitch = m_roll = m_upAxis = m_turning = QStringLiteral("-");
        emit shownChanged();
        return;
    }

    m_pitch = signedText(s.tilt.pitchDegrees()) + QStringLiteral("°");
    m_roll = signedText(s.tilt.rollDegrees()) + QStringLiteral("°");
    m_turning =
        QString::fromStdString(MMWave::DotNet::formatFixed(s.window.meanAngularRate, 1, 1)) +
        QStringLiteral(" °/s");

    // Up is in radar axes; turned back through the mounting it is what the accelerometer reads,
    // so the axis named is the one on the board's silkscreen.
    const Vector3 up = MMWave::transform(
        s.window.meanUp,
        Integration::PeopleTracking::ImuFrames::options().mounting.rotation.conjugate());
    m_upAxis = nearestAxis(up);
    emit shownChanged();
}

}  // namespace Sarwave::Core::Gui::ViewModels
