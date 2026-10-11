#pragma once

#include <QtQml/qqmlregistration.h>

#include <QObject>
#include <QPointF>
#include <QRectF>

namespace Sarwave::Core::Gui::Views::Radar {

/// Maps radar coordinates to screen coordinates for a radial (pie-slice) display.
/// Azimuth: degrees, 0 = boresight (straight up), negative = left, positive = right.
/// Cartesian: x = lateral metres (right +), y = forward metres.
/// The data overlay reuses the plot's geometry so it always lines up with the grid.
class RadarGeometry {
    Q_GADGET
    QML_VALUE_TYPE(radarGeometry)

    Q_PROPERTY(QPointF origin MEMBER origin)
    Q_PROPERTY(double scale MEMBER scale)
    Q_PROPERTY(bool isEmpty READ isEmpty)

   public:
    QPointF origin;
    double scale = 0;
    double minRange = 0;
    double maxRange = 1;
    double minAzimuth = -45;
    double maxAzimuth = 45;

    static RadarGeometry empty() {
        return {};
    }

    [[nodiscard]] bool isEmpty() const {
        return scale <= 0;
    }

    [[nodiscard]] double spanDegrees() const {
        return maxAzimuth - minAzimuth;
    }

    /// Screen position of a polar coordinate (sensor at origin).
    [[nodiscard]] QPointF fromPolar(double azimuthDegrees, double range) const;

    /// Screen position of a lateral/forward metre coordinate.
    [[nodiscard]] QPointF fromCartesian(double x, double y) const {
        return {origin.x() + scale * x, origin.y() - scale * y};
    }

    /// True when a lateral/forward metre coordinate lies inside the sector.
    [[nodiscard]] bool contains(double x, double y) const;

    /// Fits the sector (sensor apex included) into `area`, preserving the aspect ratio so that
    /// metres are the same length horizontally and vertically (arcs stay circular).
    /// Inputs are sanitised: range >= 0, outer > inner, azimuth span between 1 and 359 degrees.
    static RadarGeometry create(const QRectF& area, double minRange, double maxRange,
                                double minAzimuth, double maxAzimuth);

    friend bool operator==(const RadarGeometry&, const RadarGeometry&) = default;
};

}  // namespace Sarwave::Core::Gui::Views::Radar
