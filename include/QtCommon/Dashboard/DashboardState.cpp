#include "DashboardState.h"

#include <cmath>

DashboardState::DashboardState(const double maxRangeMetres, QObject* parent)
    : QObject(parent), m_maxRangeMetres(maxRangeMetres) {
}

bool DashboardState::presenceDetected() const {
    return m_presenceDetected;
}
QVariantList DashboardState::pointCloud() const {
    return m_pointCloud;
}
double DashboardState::centroidX() const {
    return m_centroidX;
}
double DashboardState::centroidY() const {
    return m_centroidY;
}
double DashboardState::centroidZ() const {
    return m_centroidZ;
}
double DashboardState::distance() const {
    return m_distance;
}
double DashboardState::dopplerVelocity() const {
    return m_dopplerVelocity;
}
double DashboardState::pointDensity() const {
    return m_pointDensity;
}
double DashboardState::snr() const {
    return m_snr;
}
double DashboardState::spatialSpread() const {
    return m_spatialSpread;
}
double DashboardState::classificationConfidence() const {
    return m_classificationConfidence;
}
QString DashboardState::lastUpdated() const {
    return m_lastUpdated;
}
QString DashboardState::connectionStatus() const {
    return QStringLiteral("Demo feed");
}
bool DashboardState::demoMode() const {
    return true;
}
double DashboardState::maxRangeMetres() const {
    return m_maxRangeMetres;
}

void DashboardState::updatePointCloud(const QVariantList& points) {
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

QVariantList DashboardState::drawings() const {
    return m_drawings;
}

void DashboardState::beginDrawings() {
    m_batchDrawing = true;
    m_drawings.clear();
}

void DashboardState::endDrawings() {
    m_batchDrawing = false;
    emit drawingsChanged();
}

void DashboardState::addLine(double x1, double y1, double x2, double y2, double thickness,
                             const QString& colour) {
    QVariantMap line;
    line["type"] = "line";
    line["x1"] = x1 / m_maxRangeMetres;
    line["y1"] = y1 / m_maxRangeMetres;
    line["x2"] = x2 / m_maxRangeMetres;
    line["y2"] = y2 / m_maxRangeMetres;
    line["thickness"] = thickness;
    line["colour"] = colour;
    m_drawings.append(line);
}

void DashboardState::addCircle(double cx, double cy, double radius, double thickness,
                               const QString& colour, const QString& fillColour) {
    QVariantMap circle;
    circle["type"] = "circle";
    circle["cx"] = cx / m_maxRangeMetres;
    circle["cy"] = cy / m_maxRangeMetres;
    circle["r"] = radius / m_maxRangeMetres;
    circle["thickness"] = thickness;
    circle["colour"] = colour;
    if (!fillColour.isEmpty()) {
        circle["fillColour"] = fillColour;
    }
    m_drawings.append(circle);
}

void DashboardState::clearDrawings() {
    m_drawings.clear();
}