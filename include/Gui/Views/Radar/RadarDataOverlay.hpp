#pragma once

#include <QtQml/qqmlregistration.h>

#include <QColor>
#include <QPointer>
#include <QQuickPaintedItem>

#include "Gui/ViewModels/Radar/RadarSnapshot.hpp"
#include "Gui/Views/Radar/ColourManager.hpp"
#include "Gui/Views/Radar/RadarGeometry.hpp"

namespace Sarwave::Core::Gui::Views::Radar {

/// Draws the radar data on top of RadarGridPlot: the point cloud and the tracked targets of one
/// RadarSnapshot. It owns no layout of its own; `geometry` is bound to the grid's (RadarGrid.qml)
/// so both always line up. Which colour a point or target gets is decided by `colours`.
class RadarDataOverlay : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(
        Sarwave::Core::Gui::Views::Radar::RadarGeometry geometry MEMBER m_geometry NOTIFY changed)
    Q_PROPERTY(Sarwave::Core::Gui::ViewModels::Radar::RadarSnapshotPtr snapshot MEMBER m_snapshot
                   NOTIFY changed)
    Q_PROPERTY(QColor droppedTargetBrush MEMBER m_droppedTarget NOTIFY changed)
    Q_PROPERTY(double pointRadius MEMBER m_pointRadius NOTIFY changed)
    Q_PROPERTY(double targetRadius MEMBER m_targetRadius NOTIFY changed)
    Q_PROPERTY(QString labelFontFamily MEMBER m_labelFontFamily NOTIFY changed)
    Q_PROPERTY(double labelFontSize MEMBER m_labelFontSize NOTIFY changed)

   public:
    explicit RadarDataOverlay(QQuickItem* parent = nullptr);

    /// Source of every point and active-target colour. Replace it, or change its properties,
    /// to restyle. Defaults to ColourManager::shared().
    ColourManager* colours() const {
        return m_colours;
    }
    void setColours(ColourManager* colours);

    void paint(QPainter* painter) override;

   signals:
    void changed();

   private:
    RadarGeometry m_geometry;
    ViewModels::Radar::RadarSnapshotPtr m_snapshot;
    QColor m_droppedTarget{QStringLiteral("#91a4c2")};
    double m_pointRadius = 1.5;
    double m_targetRadius = 7;
    QString m_labelFontFamily = QStringLiteral("Cascadia Mono");
    double m_labelFontSize = 9;
    QPointer<ColourManager> m_colours;
    QMetaObject::Connection m_coloursConnection;
};

}  // namespace Sarwave::Core::Gui::Views::Radar
