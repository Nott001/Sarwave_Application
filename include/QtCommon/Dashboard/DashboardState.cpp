#include "DashboardState.h"
#include <cmath>

DashboardState::DashboardState(const double maxRange, QObject* parent) : QObject(parent), m_maxRange(maxRange)
{
}

bool DashboardState::presenceDetected() const
{
    return m_presenceDetected;
}
QVariantList DashboardState::pointCloud() const
{
    return m_pointCloud;
}
double DashboardState::centroidX() const
{
    return m_centroidX;
}
double DashboardState::centroidY() const
{
    return m_centroidY;
}
double DashboardState::centroidZ() const
{
    return m_centroidZ;
}
double DashboardState::distance() const
{
    return m_distance;
}
double DashboardState::dopplerVelocity() const
{
    return m_dopplerVelocity;
}
double DashboardState::pointDensity() const
{
    return m_pointDensity;
}
double DashboardState::snr() const
{
    return m_snr;
}
double DashboardState::spatialSpread() const
{
    return m_spatialSpread;
}
double DashboardState::classificationConfidence() const
{
    return m_classificationConfidence;
}
QString DashboardState::lastUpdated() const
{
    return m_lastUpdated;
}
QString DashboardState::connectionStatus() const
{
    return QStringLiteral("Demo feed");
}
bool DashboardState::demoMode() const
{
    return true;
}
double DashboardState::maxRange() const
{
    return m_maxRange;
}

void DashboardState::updatePointCloud(const QVariantList& points)
{
    m_pointCloud = points;
    m_presenceDetected = !points.isEmpty();

    double totalDist = 0.0;
    int count = 0;
    for (const auto& p : points) {
        const QVariantMap pm = p.toMap();
        const double x = pm.value("x").toDouble();
        const double y = pm.value("y").toDouble();
        const double z = pm.value("z").toDouble();
        totalDist += std::sqrt(x * x + y * y + z * z);
        ++count;
    }
    m_distance = (count > 0) ? (totalDist / count) : 0.0;

    m_centroidX = 0.0;
    m_centroidY = 0.0;
    m_centroidZ = 0.0;
    if (count > 0) {
        for (const auto& p : points) {
            const QVariantMap pm = p.toMap();
            m_centroidX += pm.value("x").toDouble();
            m_centroidY += pm.value("y").toDouble();
            m_centroidZ += pm.value("z").toDouble();
        }
        m_centroidX /= count;
        m_centroidY /= count;
        m_centroidZ /= count;
    }

    emit detectionsChanged();
    emit pointCloudChanged();
}
