#pragma once

#include <QtQml/qqmlregistration.h>

#include <QColor>
#include <QFont>
#include <QQuickPaintedItem>

#include "Gui/Views/Radar/RadarGeometry.hpp"

namespace Sarwave::Core::Gui::Views::Radar {

/// Draws the radial radar grid only: sector fill, range arcs, azimuth rays and the unit labels in
/// the surrounding margin. It draws no data. Everything visual is a property, so colours, fonts
/// and margins can be set from QML.
/// The sector is fitted into the item minus the label margin; that margin is the "outside" area
/// where the unit markers (metres along the left edge, degrees around the outer arc) are drawn.
class RadarGridPlot : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT

    // ---- extent / subdivision -------------------------------------------------------------
    Q_PROPERTY(double minRange READ minRange WRITE setMinRange NOTIFY layoutChanged)
    Q_PROPERTY(double maxRange READ maxRange WRITE setMaxRange NOTIFY layoutChanged)
    Q_PROPERTY(double rangeStep READ rangeStep WRITE setRangeStep NOTIFY lookChanged)
    Q_PROPERTY(double minAzimuth READ minAzimuth WRITE setMinAzimuth NOTIFY layoutChanged)
    Q_PROPERTY(double maxAzimuth READ maxAzimuth WRITE setMaxAzimuth NOTIFY layoutChanged)
    Q_PROPERTY(double azimuthStep READ azimuthStep WRITE setAzimuthStep NOTIFY lookChanged)

    // ---- look -----------------------------------------------------------------------------
    Q_PROPERTY(double labelMarginLeft MEMBER m_marginLeft WRITE setMarginLeft NOTIFY layoutChanged)
    Q_PROPERTY(double labelMarginTop MEMBER m_marginTop WRITE setMarginTop NOTIFY layoutChanged)
    Q_PROPERTY(
        double labelMarginRight MEMBER m_marginRight WRITE setMarginRight NOTIFY layoutChanged)
    Q_PROPERTY(
        double labelMarginBottom MEMBER m_marginBottom WRITE setMarginBottom NOTIFY layoutChanged)
    Q_PROPERTY(QColor plotBackground MEMBER m_plotBackground NOTIFY lookChanged)
    Q_PROPERTY(QColor sectorFill MEMBER m_sectorFill NOTIFY lookChanged)
    Q_PROPERTY(QColor sectorStroke MEMBER m_sectorStroke NOTIFY lookChanged)
    Q_PROPERTY(QColor ringMajor MEMBER m_ringMajor NOTIFY lookChanged)
    Q_PROPERTY(QColor ringMinor MEMBER m_ringMinor NOTIFY lookChanged)
    Q_PROPERTY(QColor ray MEMBER m_ray NOTIFY lookChanged)
    Q_PROPERTY(QColor boresight MEMBER m_boresight NOTIFY lookChanged)
    Q_PROPERTY(QColor labelForeground MEMBER m_labelForeground NOTIFY lookChanged)
    Q_PROPERTY(QString labelFontFamily MEMBER m_labelFontFamily NOTIFY lookChanged)
    Q_PROPERTY(double labelFontSize MEMBER m_labelFontSize NOTIFY lookChanged)

    /// Current screen mapping; changes (geometryChanged) when the size or extent does.
    Q_PROPERTY(Sarwave::Core::Gui::Views::Radar::RadarGeometry geometry READ geometry NOTIFY
                   geometryChanged)

   public:
    explicit RadarGridPlot(QQuickItem* parent = nullptr);

    double minRange() const {
        return m_minRange;
    }
    void setMinRange(double v);
    double maxRange() const {
        return m_maxRange;
    }
    void setMaxRange(double v);
    double rangeStep() const {
        return m_rangeStep;
    }
    void setRangeStep(double v);
    double minAzimuth() const {
        return m_minAzimuth;
    }
    void setMinAzimuth(double v);
    double maxAzimuth() const {
        return m_maxAzimuth;
    }
    void setMaxAzimuth(double v);
    double azimuthStep() const {
        return m_azimuthStep;
    }
    void setAzimuthStep(double v);
    void setMarginLeft(double v);
    void setMarginTop(double v);
    void setMarginRight(double v);
    void setMarginBottom(double v);

    RadarGeometry geometry() const {
        return m_geometry;
    }

    void paint(QPainter* painter) override;

   signals:
    void layoutChanged();
    void lookChanged();
    void geometryChanged();

   protected:
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

   private:
    void updateGeometry();

    double m_minRange = 0;
    double m_maxRange = 6;
    double m_rangeStep = 1;
    double m_minAzimuth = -45;
    double m_maxAzimuth = 45;
    double m_azimuthStep = 15;

    double m_marginLeft = 34;
    double m_marginTop = 20;
    double m_marginRight = 34;
    double m_marginBottom = 28;
    QColor m_plotBackground{QStringLiteral("#050d18")};
    QColor m_sectorFill{QStringLiteral("#8C0F2040")};
    QColor m_sectorStroke{QStringLiteral("#1a3355")};
    QColor m_ringMajor{QStringLiteral("#122030")};
    QColor m_ringMinor{QStringLiteral("#0c1826")};
    QColor m_ray{QStringLiteral("#591A3355")};
    QColor m_boresight{QStringLiteral("#1F1FD0AA")};
    QColor m_labelForeground{QStringLiteral("#91a4c2")};
    QString m_labelFontFamily = QStringLiteral("Cascadia Mono");
    double m_labelFontSize = 9;

    RadarGeometry m_geometry;
};

}  // namespace Sarwave::Core::Gui::Views::Radar
