#include "Gui/Views/Radar/RadarGeometry.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace Sarwave::Core::Gui::Views::Radar {
namespace {
constexpr double DegToRad = std::numbers::pi / 180.0;
}

QPointF RadarGeometry::fromPolar(double azimuthDegrees, double range) const {
    const double a = azimuthDegrees * DegToRad;
    return {origin.x() + scale * range * std::sin(a), origin.y() - scale * range * std::cos(a)};
}

bool RadarGeometry::contains(double x, double y) const {
    const double range = std::sqrt(x * x + y * y);
    if (range < minRange || range > maxRange) return false;

    double azimuth = std::atan2(x, y) * 180.0 / std::numbers::pi;
    if (azimuth < minAzimuth) azimuth += 360;  // sectors whose right edge passes 180 degrees
    return azimuth >= minAzimuth && azimuth <= maxAzimuth;
}

RadarGeometry RadarGeometry::create(const QRectF& area, double minRange, double maxRange,
                                    double minAzimuth, double maxAzimuth) {
    minRange = std::max(0.0, minRange);
    maxRange = std::max(maxRange, minRange + 0.01);
    minAzimuth = std::clamp(minAzimuth, -180.0, 180.0);
    maxAzimuth = std::clamp(maxAzimuth, minAzimuth + 1, minAzimuth + 359);

    double x0 = 0, x1 = 0, y0 = 0, y1 = 0;  // bounding box in metres, starts at the sensor
    const auto add = [&](double azimuth, double r) {
        const double a = azimuth * DegToRad;
        const double x = r * std::sin(a);
        const double y = r * std::cos(a);
        x0 = std::min(x0, x);
        x1 = std::max(x1, x);
        y0 = std::min(y0, y);
        y1 = std::max(y1, y);
    };

    add(minAzimuth, maxRange);
    add(maxAzimuth, maxRange);
    // The arc can bulge past its end points wherever it crosses 0/90/180/270 degrees.
    for (auto k = static_cast<int>(std::ceil(minAzimuth / 90.0));
         k <= static_cast<int>(std::floor(maxAzimuth / 90.0)); k++) {
        add(k * 90.0, maxRange);
    }

    const double bw = x1 - x0;
    const double bh = y1 - y0;
    RadarGeometry g;
    g.minRange = minRange;
    g.maxRange = maxRange;
    g.minAzimuth = minAzimuth;
    g.maxAzimuth = maxAzimuth;
    if (area.width() <= 0 || area.height() <= 0 || bw <= 0 || bh <= 0) return g;

    g.scale = std::min(area.width() / bw, area.height() / bh);
    g.origin = {area.x() + (area.width() - g.scale * bw) / 2 - g.scale * x0,
                area.y() + (area.height() - g.scale * bh) / 2 + g.scale * y1};
    return g;
}

}  // namespace Sarwave::Core::Gui::Views::Radar
