#include "Gui/Views/Radar/RadarDataOverlay.hpp"

#include <QFontMetricsF>
#include <QPainter>
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

void RadarDataOverlay::paint(QPainter* painter) {
    const RadarGeometry& g = m_geometry;
    if (g.isEmpty() || !m_snapshot || !m_colours) return;

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    const ColourManager& colours = *m_colours;

    // Point clouds (under the targets), oldest first so newer points land on top. Each cloud gets
    // more transparent as its fade goes from 0 to 1. Within a cloud the rejected points come
    // first, so a target's points are drawn over them. Points outside the sector are skipped, not
    // clamped.
    const double r = m_pointRadius;
    painter->setPen(Qt::NoPen);
    for (const auto& cloud : m_snapshot->pointClouds()) {
        painter->setOpacity(1.0 - cloud.fade);
        for (const auto& group : cloud.groups) {
            painter->setBrush(colours.forPoints(group.objectId));
            for (const auto& p : group.points) {
                if (!g.contains(p.x, p.y)) continue;
                painter->drawEllipse(g.fromCartesian(p.x, p.y), r, r);
            }
        }
    }
    painter->setOpacity(1.0);

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

}  // namespace Sarwave::Core::Gui::Views::Radar
