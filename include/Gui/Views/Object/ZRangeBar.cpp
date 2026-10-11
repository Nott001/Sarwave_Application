#include "Gui/Views/Object/ZRangeBar.hpp"

#include <QPainter>
#include <algorithm>
#include <cmath>

namespace Sarwave::Core::Gui::Views::Object {

ZRangeBar::ZRangeBar(QQuickItem* parent) : QQuickPaintedItem(parent) {
    setAntialiasing(true);
    connect(this, &ZRangeBar::changed, this, [this] { update(); });
}

void ZRangeBar::paint(QPainter* painter) {
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);

    const bool vertical = m_orientation == Qt::Vertical;
    // Everything below is worked out along the scale ("length") and across it ("thickness"), then
    // turned into a screen rectangle by along().
    const double length = vertical ? height() : width();
    const double thickness = vertical ? width() : height();
    const double span = m_maximum - m_minimum;
    if (length <= 0 || thickness <= 0 || !(span > 0)) return;

    // The stretch [a, b] of the scale, measured in pixels from the minimum end.
    const auto along = [&](double a, double b) {
        return vertical ? QRectF(0, length - b, thickness, b - a) : QRectF(a, 0, b - a, thickness);
    };

    const double radius = thickness / 2;
    painter->setBrush(m_track);
    painter->drawRoundedRect(along(0, length), radius, radius);

    // Ticks first, so the filled part covers the ones it passes.
    const double step = m_tickStep;
    if (step > 0 && span / step <= 200 && m_tick.isValid()) {
        painter->setBrush(m_tick);
        for (double v = m_minimum + step; v < m_maximum - step / 1000; v += step) {
            const double at = std::nearbyint((v - m_minimum) / span * length);
            QRectF tick = along(at, at + 1);
            tick = vertical ? tick.adjusted(2, 0, -2, 0) : tick.adjusted(0, 2, 0, -2);
            painter->drawRect(tick);
        }
    }

    const double low = std::min(m_low, m_high);
    const double high = std::max(m_low, m_high);
    if (std::isnan(low) || std::isnan(high)) return;

    double from = std::clamp((low - m_minimum) / span, 0.0, 1.0) * length;
    double to = std::clamp((high - m_minimum) / span, 0.0, 1.0) * length;

    // Never shorter than the bar is thick, so a just-created track (min ~ max) still shows.
    if (to - from < thickness && length >= thickness) {
        const double mid = std::clamp((from + to) / 2, thickness / 2, length - thickness / 2);
        from = mid - thickness / 2;
        to = mid + thickness / 2;
    }

    painter->setBrush(m_fill);
    painter->drawRoundedRect(along(from, to), radius, radius);
}

}  // namespace Sarwave::Core::Gui::Views::Object
