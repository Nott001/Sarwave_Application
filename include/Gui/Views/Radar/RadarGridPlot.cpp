#include "Gui/Views/Radar/RadarGridPlot.hpp"

#include <QFontMetricsF>
#include <QPainter>
#include <QPainterPath>
#include <algorithm>
#include <cmath>
#include <numbers>

#include "MMWave/Common/DotNetFormat.hpp"

namespace Sarwave::Core::Gui::Views::Radar {
namespace {

QPen makePen(const QColor& colour, double thickness) {
    QPen pen(colour, thickness);
    pen.setCapStyle(Qt::FlatCap);  // Avalonia's Pen defaults
    pen.setJoinStyle(Qt::MiterJoin);
    return pen;
}

QString format(double v) {
    return QString::fromStdString(MMWave::DotNet::formatFixed(v, 0, 2));  // "0.##"
}

/// The circle of radius `range` around the sensor, for QPainterPath::arcTo.
QRectF circle(const RadarGeometry& g, double range) {
    const double r = g.scale * range;
    return {g.origin.x() - r, g.origin.y() - r, 2 * r, 2 * r};
}

// Avalonia arcs sweep clockwise on screen from MinAzimuth to MaxAzimuth (azimuth 0 = up). Qt
// measures angles counter-clockwise from 3 o'clock, so azimuth a is at 90 - a, swept negatively.
void arcClockwise(QPainterPath& path, const RadarGeometry& g, double range) {
    path.arcTo(circle(g, range), 90.0 - g.minAzimuth, -g.spanDegrees());
}

/// Pie slice (or annular slice when MinRange > 0). Azimuth increases clockwise on screen.
QPainterPath sectorShape(const RadarGeometry& g) {
    QPainterPath path;
    if (g.minRange <= 0) {
        path.moveTo(g.origin);
        path.lineTo(g.fromPolar(g.minAzimuth, g.maxRange));
        arcClockwise(path, g, g.maxRange);
    } else {
        path.moveTo(g.fromPolar(g.minAzimuth, g.minRange));
        path.lineTo(g.fromPolar(g.minAzimuth, g.maxRange));
        arcClockwise(path, g, g.maxRange);
        path.lineTo(g.fromPolar(g.maxAzimuth, g.minRange));
        path.arcTo(circle(g, g.minRange), 90.0 - g.maxAzimuth, g.spanDegrees());
    }
    path.closeSubpath();
    return path;
}

QPainterPath arcShape(const RadarGeometry& g, double range) {
    QPainterPath path;
    path.moveTo(g.fromPolar(g.minAzimuth, range));
    arcClockwise(path, g, range);
    return path;
}

}  // namespace

RadarGridPlot::RadarGridPlot(QQuickItem* parent) : QQuickPaintedItem(parent) {
    setAntialiasing(true);
    setClip(true);
    connect(this, &RadarGridPlot::lookChanged, this, [this] { update(); });
    connect(this, &RadarGridPlot::layoutChanged, this, [this] {
        updateGeometry();
        update();
    });
}

#define SARWAVE_SETTER(Name, member, signal) \
    void RadarGridPlot::Name(double v) {     \
        if (member == v) return;             \
        member = v;                          \
        emit signal();                       \
    }

SARWAVE_SETTER(setMinRange, m_minRange, layoutChanged)
SARWAVE_SETTER(setMaxRange, m_maxRange, layoutChanged)
SARWAVE_SETTER(setRangeStep, m_rangeStep, lookChanged)
SARWAVE_SETTER(setMinAzimuth, m_minAzimuth, layoutChanged)
SARWAVE_SETTER(setMaxAzimuth, m_maxAzimuth, layoutChanged)
SARWAVE_SETTER(setAzimuthStep, m_azimuthStep, lookChanged)
SARWAVE_SETTER(setMarginLeft, m_marginLeft, layoutChanged)
SARWAVE_SETTER(setMarginTop, m_marginTop, layoutChanged)
SARWAVE_SETTER(setMarginRight, m_marginRight, layoutChanged)
SARWAVE_SETTER(setMarginBottom, m_marginBottom, layoutChanged)
#undef SARWAVE_SETTER

void RadarGridPlot::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) {
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
    if (newGeometry.size() != oldGeometry.size()) updateGeometry();
}

void RadarGridPlot::updateGeometry() {
    const QRectF area(m_marginLeft, m_marginTop,
                      std::max(0.0, width() - m_marginLeft - m_marginRight),
                      std::max(0.0, height() - m_marginTop - m_marginBottom));
    const RadarGeometry g =
        RadarGeometry::create(area, m_minRange, m_maxRange, m_minAzimuth, m_maxAzimuth);
    if (g == m_geometry) return;
    m_geometry = g;
    emit geometryChanged();
}

void RadarGridPlot::paint(QPainter* painter) {
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    painter->fillRect(QRectF(0, 0, width(), height()), m_plotBackground);

    const RadarGeometry& g = m_geometry;
    if (g.isEmpty()) return;

    const double rangeStep = std::max(m_rangeStep, g.maxRange / 100.0);
    const double azimuthStep = std::max(m_azimuthStep, 1.0);

    // Sector fill (under everything else).
    const QPainterPath sector = sectorShape(g);
    painter->fillPath(sector, m_sectorFill);

    // Range arcs.
    const int firstRing = std::max(1, static_cast<int>(std::ceil(g.minRange / rangeStep - 1e-9)));
    for (int k = firstRing; k * rangeStep <= g.maxRange + 1e-9; k++) {
        const bool even = k % 2 == 0;
        painter->strokePath(arcShape(g, k * rangeStep),
                            makePen(even ? m_ringMajor : m_ringMinor, even ? 0.9 : 0.5));
    }

    // Azimuth rays (boresight highlighted).
    const int firstRay = static_cast<int>(std::ceil(g.minAzimuth / azimuthStep - 1e-9));
    const int lastRay = static_cast<int>(std::floor(g.maxAzimuth / azimuthStep + 1e-9));
    for (int k = firstRay; k <= lastRay; k++) {
        const double az = k * azimuthStep;
        painter->setPen(az == 0 ? makePen(m_boresight, 0.8) : makePen(m_ray, 0.5));
        painter->drawLine(g.fromPolar(az, g.minRange), g.fromPolar(az, g.maxRange));
    }

    // Sector outline on top.
    painter->strokePath(sector, makePen(m_sectorStroke, 0.9));

    // Unit markers, outside the grid area.
    QFont font(m_labelFontFamily);
    font.setPixelSize(std::max(1, static_cast<int>(std::lround(m_labelFontSize))));
    painter->setFont(font);
    painter->setPen(m_labelForeground);
    const QFontMetricsF metrics(font);

    // Draws the text centred on anchor + direction * (gap + half the text's extent along that
    // direction).
    const auto drawCentred = [&](const QString& text, QPointF anchor, QPointF direction) {
        constexpr double gap = 5;
        const double w = metrics.horizontalAdvance(text);
        const double h = metrics.lineSpacing();
        const double offset =
            gap + w / 2 * std::abs(direction.x()) + h / 2 * std::abs(direction.y());
        const QPointF centre = anchor + direction * offset;
        painter->drawText(QPointF(centre.x() - w / 2, centre.y() - h / 2 + metrics.ascent()), text);
    };

    // Degrees: just beyond the outer arc, pushed out along each ray.
    for (int k = firstRay; k <= lastRay; k++) {
        const double az = k * azimuthStep;
        const double a = az * std::numbers::pi / 180.0;
        drawCentred(format(az) + QStringLiteral("°"), g.fromPolar(az, g.maxRange),
                    QPointF(std::sin(a), -std::cos(a)));
    }

    // Metres: along the left edge, pushed out perpendicular to it.
    {
        const double a = g.minAzimuth * std::numbers::pi / 180.0;
        const QPointF normal(-std::cos(a), -std::sin(a));  // points away from the sector
        for (int k = firstRing; k * rangeStep <= g.maxRange + 1e-9; k++) {
            const double r = k * rangeStep;
            drawCentred(format(r) + QStringLiteral("m"), g.fromPolar(g.minAzimuth, r), normal);
        }
    }
}

}  // namespace Sarwave::Core::Gui::Views::Radar
