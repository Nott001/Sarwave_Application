#include "Gui/ViewModels/Object/ObjectItemViewModel.hpp"

#include <cmath>
#include <numbers>

#include "Integration/PeopleTracking/Association/TargetObject.hpp"
#include "MMWave/Common/DotNetFormat.hpp"

namespace Sarwave::Core::Gui::ViewModels::Object {
namespace {

QString fixed2(float value) {
    return QString::fromStdString(MMWave::DotNet::formatFixed(value, 2, 2));
}

constexpr float RadToDeg = 180.0f / std::numbers::pi_v<float>;

}  // namespace

ObjectItemViewModel::ObjectItemViewModel(QObject* parent)
    : ObjectItemViewModel(
          static_cast<int>(
              Integration::PeopleTracking::Association::NonObjectPoint::PointNotYetAssigned),
          parent) {
}

ObjectItemViewModel::ObjectItemViewModel(int objectId, QObject* parent)
    : ViewModelBase(parent), m_objectId(objectId) {
}

QString ObjectItemViewModel::idText() const {
    return QStringLiteral("#") + QString::number(m_objectId);
}

void ObjectItemViewModel::setIsActive(bool value) {
    if (m_isActive == value) return;
    m_isActive = value;
    emit isActiveChanged();
}

void ObjectItemViewModel::setConfidenceLevel(float value) {
    if (m_confidenceLevel == value) return;
    m_confidenceLevel = value;
    emit confidenceLevelChanged();
}

void ObjectItemViewModel::setPosition(MMWave::Vector3 value) {
    if (m_position == value) return;
    m_position = value;
    emit positionChanged();
}

void ObjectItemViewModel::setVelocity(MMWave::Vector3 value) {
    if (m_velocity == value) return;
    m_velocity = value;
    emit velocityChanged();
}

void ObjectItemViewModel::setAcceleration(MMWave::Vector3 value) {
    if (m_acceleration == value) return;
    m_acceleration = value;
    emit accelerationChanged();
}

void ObjectItemViewModel::setMinZ(float value) {
    if (m_minZ == value) return;
    m_minZ = value;
    emit minZChanged();
    emit heightChanged();
}

void ObjectItemViewModel::setMaxZ(float value) {
    if (m_maxZ == value) return;
    m_maxZ = value;
    emit maxZChanged();
    emit heightChanged();
}

QString ObjectItemViewModel::confidenceText() const {
    return QString::number(confidencePercent()) + QStringLiteral("%");
}

float ObjectItemViewModel::distance() const {
    return std::sqrt(m_position.x * m_position.x + m_position.y * m_position.y);
}

float ObjectItemViewModel::azimuth() const {
    return std::atan2(m_position.x, m_position.y) * RadToDeg;
}

float ObjectItemViewModel::heading() const {
    return std::atan2(m_velocity.x, m_velocity.y) * RadToDeg;
}

QString ObjectItemViewModel::heightText() const {
    return fixed2(height());
}

QString ObjectItemViewModel::minZText() const {
    return fixed2(m_minZ);
}

QString ObjectItemViewModel::maxZText() const {
    return fixed2(m_maxZ);
}

}  // namespace Sarwave::Core::Gui::ViewModels::Object
