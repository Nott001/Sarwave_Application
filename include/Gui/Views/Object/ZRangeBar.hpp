#pragma once

#include <QtQml/qqmlregistration.h>

#include <QColor>
#include <QQuickPaintedItem>

namespace Sarwave::Core::Gui::Views::Object {

/// A scale from `minimum` to `maximum` with the stretch from `low` to `high` filled in: where a
/// target's lowest and highest points sit, and (as the length of the filled part) how tall it
/// is. Values outside the scale are clamped to its ends. Vertical puts `minimum` at the bottom;
/// horizontal puts it on the left. Pure presentation; give it a width and height in QML.
class ZRangeBar : public QQuickPaintedItem {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(Qt::Orientation orientation MEMBER m_orientation NOTIFY changed)
    /// Value at the bottom (vertical) or left (horizontal) end of the scale.
    Q_PROPERTY(double minimum MEMBER m_minimum NOTIFY changed)
    /// Value at the top (vertical) or right (horizontal) end of the scale.
    Q_PROPERTY(double maximum MEMBER m_maximum NOTIFY changed)
    Q_PROPERTY(double low MEMBER m_low NOTIFY changed)
    Q_PROPERTY(double high MEMBER m_high NOTIFY changed)
    /// Distance between tick marks, in scale units. 0 or less = no ticks.
    Q_PROPERTY(double tickStep MEMBER m_tickStep NOTIFY changed)
    Q_PROPERTY(QColor trackBrush MEMBER m_track NOTIFY changed)
    Q_PROPERTY(QColor fillBrush MEMBER m_fill NOTIFY changed)
    Q_PROPERTY(QColor tickBrush MEMBER m_tick NOTIFY changed)

   public:
    explicit ZRangeBar(QQuickItem* parent = nullptr);

    void paint(QPainter* painter) override;

   signals:
    void changed();

   private:
    Qt::Orientation m_orientation = Qt::Vertical;
    double m_minimum = 0;
    double m_maximum = 2.5;
    double m_low = 0;
    double m_high = 0;
    double m_tickStep = 0.5;
    QColor m_track{QStringLiteral("#0c1826")};
    QColor m_fill{QStringLiteral("#b9a5ff")};
    QColor m_tick{QStringLiteral("#1e3052")};
};

}  // namespace Sarwave::Core::Gui::Views::Object
