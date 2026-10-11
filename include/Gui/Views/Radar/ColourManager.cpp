#include "Gui/Views/Radar/ColourManager.hpp"

#include "MMWave/Configured/PeopleTracking/Association/TargetObject.hpp"

namespace Sarwave::Core::Gui::Views::Radar {

using MMWave::Configured::PeopleTracking::Association::isNonObjectPoint;
using MMWave::Configured::PeopleTracking::Association::NonObjectPoint;

ColourManager::ColourManager(QObject* parent) : QObject(parent) {
    for (const char* hex :
         {"#3dff6d", "#3dff7e", "#3dff8e", "#3dff9e", "#3dffae", "#3dffbe", "#3dffce", "#3dffdf"}) {
        m_palette.append(QColor(QString::fromLatin1(hex)));
    }
}

ColourManager& ColourManager::shared() {
    static ColourManager instance;
    return instance;
}

void ColourManager::setDefaultColour(const QColor& value) {
    m_default = value;
    emit changed();
}

void ColourManager::setNoise(const QColor& value) {
    m_noise = value;
    emit changed();
}

void ColourManager::setUnassigned(const QColor& value) {
    m_unassigned = value;
    emit changed();
}

void ColourManager::setPalette(const QList<QColor>& value) {
    m_palette = value;
    emit changed();
}

QColor ColourManager::fromPalette(int index) const {
    const auto count = static_cast<int>(m_palette.size());
    if (count == 0) return m_default;
    return m_palette[(index % count + count) % count];
}

QColor ColourManager::forPoints(int objectId) const {
    if (objectId == static_cast<int>(NonObjectPoint::PointNotYetAssigned)) return m_unassigned;
    return isNonObjectPoint(objectId) ? m_noise : forTarget(objectId);
}

}  // namespace Sarwave::Core::Gui::Views::Radar
