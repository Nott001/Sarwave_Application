#include "Gui/Views/Radar/RadarDataOverlay.hpp"

#include <QFontMetricsF>
#include <QPainter>
#include <chrono>
#include <cmath>

namespace Sarwave::Core::Gui::Views::Radar {

RadarDataOverlay::RadarDataOverlay(QQuickItem* parent) : QQuickPaintedItem(parent) {
    setAntialiasing(true);
    setClip(true);
    setAcceptedMouseButtons(Qt::NoButton);  // IsHitTestVisible="False"
    connect(this, &RadarDataOverlay::changed, this, [this] { update(); });
    setColours(&ColourManager::shared());
}

void RadarDataOverlay::setColours(ColourManager* colours) {
    disconnect(m_coloursConnection);
    m_colours = colours;
    if (colours)
        m_coloursConnection = connect(colours, &ColourManager::changed, this, [this] { update(); });
    update();
}

MMWave::DurationStats& RadarDataOverlay::paintTimes() {
    static MMWave::DurationStats times;
    return times;
}

void RadarDataOverlay::paint(QPainter* painter) {
    const auto start = std::chrono::steady_clock::now();
    paintData(painter);
    paintTimes().add(std::chrono::steady_clock::now() - start);
}

void RadarDataOverlay::paintData(QPainter* painter) {
    const RadarGeometry& g = m_geometry;
    if (g.isEmpty() || !m_snapshot || !m_colours) return;

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    const ColourManager& colours = *m_colours;

    paintPoints(painter);

    // Targets: ring + centre dot + object id, in the same colour as their points.
    QFont font(m_labelFontFamily);
    font.setPixelSize(std::max(1, static_cast<int>(std::lround(m_labelFontSize))));
    painter->setFont(font);
    const QFontMetricsF metrics(font);

    for (const auto& target : m_snapshot->targets()) {
        const QColor colour =
            target.isActive ? colours.forTarget(target.objectId) : m_droppedTarget;
        if (!colour.isValid() || !g.contains(target.position.x, target.position.y)) continue;

        const QPointF centre = g.fromCartesian(target.position.x, target.position.y);
        const double radius = m_targetRadius;

        QPen ring(colour, 1.5);
        ring.setCapStyle(Qt::FlatCap);
        painter->setPen(ring);
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(centre, radius, radius);

        painter->setPen(Qt::NoPen);
        painter->setBrush(colour);
        painter->drawEllipse(centre, 2.0, 2.0);

        const QString text = QStringLiteral("#") + QString::number(target.objectId);
        const double h = metrics.lineSpacing();
        painter->setPen(colour);
        painter->drawText(QPointF(centre.x() + radius + 3, centre.y() - h / 2 + metrics.ascent()),
                          text);
    }
}

// Point clouds (under the targets), oldest first so newer points land on top. Each cloud gets more
// transparent as its fade goes from 0 to 1. Within a cloud the rejected points come first, so a
// target's points are drawn over them. Points outside the sector are skipped, not clamped.
void RadarDataOverlay::paintPoints(QPainter* painter) {
    // The sprites are copied in device pixels, which needs a transform that only moves and
    // scales evenly (always the case for this item). Anything else gets the ellipses.
    const QTransform device = painter->worldTransform();
    if (device.type() > QTransform::TxScale || device.m11() != device.m22() || device.m11() <= 0) {
        paintPointsSlow(painter);
        return;
    }

    const RadarGeometry& g = m_geometry;
    const double deviceRadius = m_pointRadius * device.m11();

    painter->save();
    painter->resetTransform();
    for (const auto& cloud : m_snapshot->pointClouds()) {
        painter->setOpacity(1.0 - cloud.fade);
        for (const auto& group : cloud.groups) {
            const DotSprites& dots = dotSprites(m_colours->forPoints(group.objectId), deviceRadius);
            for (const auto& p : group.points) {
                if (!g.contains(p.x, p.y)) continue;

                const QPointF at = device.map(g.fromCartesian(p.x, p.y));
                const double x = at.x() - dots.offset;
                const double y = at.y() - dots.offset;
                auto left = static_cast<int>(std::floor(x));
                auto top = static_cast<int>(std::floor(y));
                auto phaseX = static_cast<int>(std::lround((x - left) * DotPhases));
                auto phaseY = static_cast<int>(std::lround((y - top) * DotPhases));
                if (phaseX == DotPhases) phaseX = 0, left++;
                if (phaseY == DotPhases) phaseY = 0, top++;

                painter->drawImage(
                    QPoint(left, top),
                    dots.phases[static_cast<std::size_t>(phaseY * DotPhases + phaseX)]);
            }
        }
    }
    painter->restore();
}

void RadarDataOverlay::paintPointsSlow(QPainter* painter) {
    const RadarGeometry& g = m_geometry;
    const double r = m_pointRadius;
    painter->setPen(Qt::NoPen);
    for (const auto& cloud : m_snapshot->pointClouds()) {
        painter->setOpacity(1.0 - cloud.fade);
        for (const auto& group : cloud.groups) {
            painter->setBrush(m_colours->forPoints(group.objectId));
            for (const auto& p : group.points) {
                if (!g.contains(p.x, p.y)) continue;
                painter->drawEllipse(g.fromCartesian(p.x, p.y), r, r);
            }
        }
    }
    painter->setOpacity(1.0);
}

const RadarDataOverlay::DotSprites& RadarDataOverlay::dotSprites(const QColor& colour,
                                                                 double deviceRadius) {
    // A new radius (zoom, another screen) or a palette that keeps changing: start over.
    if (deviceRadius != m_dotSpriteRadius || m_dotSprites.size() > 64) {
        m_dotSprites.clear();
        m_dotSpriteRadius = deviceRadius;
    }

    const QRgb rgba = colour.rgba();
    for (const DotSprites& dots : m_dotSprites) {
        if (dots.colour == rgba) return dots;
    }

    // One pixel of margin on each side, plus one for the sub-pixel shift.
    const int size = static_cast<int>(std::ceil(2 * deviceRadius)) + 3;
    DotSprites& dots = m_dotSprites.emplace_back();
    dots.colour = rgba;
    dots.offset = std::floor(size / 2.0) - 0.5;
    for (int py = 0; py < DotPhases; py++) {
        for (int px = 0; px < DotPhases; px++) {
            QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
            QPainter p(&image);
            p.setRenderHint(QPainter::Antialiasing, true);
            p.setPen(Qt::NoPen);
            p.setBrush(colour);
            p.drawEllipse(QPointF(dots.offset + static_cast<double>(px) / DotPhases,
                                  dots.offset + static_cast<double>(py) / DotPhases),
                          deviceRadius, deviceRadius);
            p.end();
            dots.phases[static_cast<std::size_t>(py * DotPhases + px)] = std::move(image);
        }
    }
    return dots;
}

}  // namespace Sarwave::Core::Gui::Views::Radar
