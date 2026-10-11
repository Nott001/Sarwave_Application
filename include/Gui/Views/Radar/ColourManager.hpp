#pragma once

#include <QtQml/qqmlregistration.h>

#include <QColor>
#include <QList>
#include <QObject>

namespace Sarwave::Core::Gui::Views::Radar {

/// Decides which colour a radar point or target is drawn in. Every colour is a settable
/// property, so a host can restyle the display without touching the drawing code.
class ColourManager : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use ColourManager::shared().")

    Q_PROPERTY(QColor defaultColour READ defaultColour WRITE setDefaultColour NOTIFY changed)
    Q_PROPERTY(QColor noise READ noise WRITE setNoise NOTIFY changed)
    Q_PROPERTY(QColor unassigned READ unassigned WRITE setUnassigned NOTIFY changed)
    Q_PROPERTY(QList<QColor> palette READ palette WRITE setPalette NOTIFY changed)

   public:
    explicit ColourManager(QObject* parent = nullptr);

    /// The instance the radar overlay and TargetColourConverter use unless given another one.
    /// Change its properties to restyle every display that shows targets at once.
    static ColourManager& shared();

    /// Fallback for anything that has no more specific colour.
    [[nodiscard]] QColor defaultColour() const {
        return m_default;
    }
    void setDefaultColour(const QColor& value);

    /// Points the tracker looked at and rejected: noise, weak SNR, out of bounds (the
    /// NonObjectPoint ids).
    [[nodiscard]] QColor noise() const {
        return m_noise;
    }
    void setNoise(const QColor& value);

    /// Points the tracker has not classified yet: their cloud is still waiting for the next
    /// frame's Target Index TLV.
    [[nodiscard]] QColor unassigned() const {
        return m_unassigned;
    }
    void setUnassigned(const QColor& value);

    /// Colours handed out to targets. Object ids only ever count up, so the palette is walked
    /// in order and starts over once it runs out.
    [[nodiscard]] QList<QColor> palette() const {
        return m_palette;
    }
    void setPalette(const QList<QColor>& value);

    /// The palette colour at `index`, wrapping around in both directions. defaultColour if the
    /// palette is empty.
    Q_INVOKABLE QColor fromPalette(int index) const;

    /// Colour of a target and of the points that belong to it.
    Q_INVOKABLE QColor forTarget(int objectId) const {
        return fromPalette(objectId);
    }

    /// Colour for a group of points by the object id they carry: unassigned for
    /// PointNotYetAssigned, noise for the other NonObjectPoint ids, otherwise the target's.
    Q_INVOKABLE QColor forPoints(int objectId) const;

   signals:
    void changed();

   private:
    QColor m_default{QStringLiteral("#73a9ff")};
    QColor m_noise{QStringLiteral("#4a5a75")};
    QColor m_unassigned{QStringLiteral("#91a4c2")};
    QList<QColor> m_palette;
};

/// The "#id" colour of the objects panel: the colour the target and its points have on the radar
/// plot. (Referenced as radar:TargetColourConverter.Instance by ObjectItem.axaml.)
class TargetColourConverter : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

   public:
    using QObject::QObject;

    Q_INVOKABLE QColor convert(int objectId) const {
        return ColourManager::shared().forTarget(objectId);
    }
};

}  // namespace Sarwave::Core::Gui::Views::Radar
