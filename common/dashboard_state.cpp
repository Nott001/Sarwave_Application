#include "dashboard_state.h"

#include <QDateTime>
#include <QTimer>

#include <cmath>

DashboardState::DashboardState(QObject *parent)
    : QObject(parent), m_refreshTimer(new QTimer(this)) {
    m_refreshTimer->setInterval(2'000);
    connect(m_refreshTimer, &QTimer::timeout, this, &DashboardState::updateDemoReadings);
    updateDemoReadings();
    m_refreshTimer->start();
}

double DashboardState::temperature() const { return m_temperature; }
double DashboardState::humidity() const { return m_humidity; }
double DashboardState::pressure() const { return m_pressure; }
QString DashboardState::lastUpdated() const { return m_lastUpdated; }
QString DashboardState::connectionStatus() const { return QStringLiteral("Demo feed"); }
bool DashboardState::demoMode() const { return true; }

void DashboardState::refresh() {
    updateDemoReadings();
}

void DashboardState::updateDemoReadings() {
    ++m_sample;
    const double phase = static_cast<double>(m_sample) / 5.0;
    m_temperature = 24.8 + std::sin(phase) * 0.7;
    m_humidity = 58.4 + std::cos(phase * 0.7) * 2.1;
    m_pressure = 1012.6 + std::sin(phase * 0.45) * 1.3;
    m_lastUpdated = QDateTime::currentDateTimeUtc().toString(QStringLiteral("HH:mm:ss 'UTC'"));
    emit readingsChanged();
}
