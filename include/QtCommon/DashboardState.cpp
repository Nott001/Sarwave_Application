#include "DashboardState.h"

DashboardState::DashboardState(QObject *parent)
    : QObject(parent) {
}

bool DashboardState::presenceDetected() const { return m_presenceDetected; }
QVariantList DashboardState::pointCloud() const { return m_pointCloud; }
double DashboardState::centroidX() const { return m_centroidX; }
double DashboardState::centroidY() const { return m_centroidY; }
double DashboardState::centroidZ() const { return m_centroidZ; }
double DashboardState::distance() const { return m_distance; }
double DashboardState::dopplerVelocity() const { return m_dopplerVelocity; }
double DashboardState::pointDensity() const { return m_pointDensity; }
double DashboardState::snr() const { return m_snr; }
double DashboardState::spatialSpread() const { return m_spatialSpread; }
double DashboardState::classificationConfidence() const { return m_classificationConfidence; }
QString DashboardState::lastUpdated() const { return m_lastUpdated; }
QString DashboardState::connectionStatus() const { return QStringLiteral("Demo feed"); }
bool DashboardState::demoMode() const { return true; }

void DashboardState::updatePointCloud(const QVariantList &points) {
    m_pointCloud = points;
    m_presenceDetected = !points.isEmpty();
    emit detectionsChanged();
    emit pointCloudChanged();
}