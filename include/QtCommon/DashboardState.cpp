#include "DashboardState.h"

#include <QDateTime>
#include <QTimer>
#include <cmath>

DashboardState::DashboardState(QObject *parent)
    : QObject(parent), m_refreshTimer(new QTimer(this)) {
    m_refreshTimer->setInterval(2'000);
    connect(m_refreshTimer, &QTimer::timeout, this, &DashboardState::updateDemoDetections);
    updateDemoDetections();
    m_refreshTimer->start();
}

bool DashboardState::presenceDetected() const { return m_presenceDetected; }
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

void DashboardState::refresh() {
    updateDemoDetections();
}

void DashboardState::updateDemoDetections() {
    ++m_sample;
    const double phase = static_cast<double>(m_sample) / 5.0;
    m_presenceDetected = (std::sin(phase * 0.3) > -0.3);

    // Lateral position: oscillates ±1.2 m, keeping target inside ±45° FOV
    m_centroidX = std::sin(phase * 0.2) * 1.2;

    // Forward (range) position: always positive — target is always in front of
    // the sensor.  std::abs() ensures centroidY never goes negative.
    m_centroidY = std::abs(std::cos(phase * 0.15)) * 1.5 + 0.8;  // 0.8–2.3 m

    // Height: kept shallow so 3D Euclidean distance stays well within 5 m
    m_centroidZ = std::sin(phase * 0.1) * 0.3 + 0.5;             // 0.2–0.8 m

    m_distance = std::sqrt(m_centroidX * m_centroidX +
                           m_centroidY * m_centroidY +
                           m_centroidZ * m_centroidZ);
    m_dopplerVelocity = 0.05 + std::sin(phase * 0.4) * 0.03;
    m_pointDensity    = 150.0 + std::sin(phase) * 50.0;
    m_snr             = 12.0 + std::cos(phase * 0.6) * 3.0;
    m_spatialSpread   = 0.4 + std::sin(phase * 0.25) * 0.15;
    m_classificationConfidence = 0.85 + std::sin(phase * 0.35) * 0.1;
    m_lastUpdated = QDateTime::currentDateTimeUtc().toString(QStringLiteral("HH:mm:ss 'UTC'"));
    emit detectionsChanged();
}
