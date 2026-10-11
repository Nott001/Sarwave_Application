#pragma once

#include <QString>
#include <QVector3D>

#include "Gui/ViewModels/ViewModelBase.hpp"
#include "MMWave/Common/Numerics.hpp"

namespace Sarwave::Core::Gui::ViewModels::Object {

/// One tracked target as a row in the objects panel. The stored properties are copied from the
/// target each frame; everything under "derived" is computed from them and follows automatically.
///
/// The *Text properties are the AXAML StringFormat bindings, formatted the way .NET formats them
/// ("{0:0.00}" of a float, "{0}%", "#{0}").
class ObjectItemViewModel : public ViewModelBase {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by ObjectsPanelViewModel.")

    Q_PROPERTY(int objectId READ objectId CONSTANT)
    Q_PROPERTY(QString idText READ idText CONSTANT)
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(float confidenceLevel READ confidenceLevel NOTIFY confidenceLevelChanged)
    Q_PROPERTY(int confidencePercent READ confidencePercent NOTIFY confidenceLevelChanged)
    Q_PROPERTY(QString confidenceText READ confidenceText NOTIFY confidenceLevelChanged)
    Q_PROPERTY(QVector3D position READ positionQt NOTIFY positionChanged)
    Q_PROPERTY(QVector3D velocity READ velocityQt NOTIFY velocityChanged)
    Q_PROPERTY(QVector3D acceleration READ accelerationQt NOTIFY accelerationChanged)
    Q_PROPERTY(float minZ READ minZ NOTIFY minZChanged)
    Q_PROPERTY(float maxZ READ maxZ NOTIFY maxZChanged)
    Q_PROPERTY(QString minZText READ minZText NOTIFY minZChanged)
    Q_PROPERTY(QString maxZText READ maxZText NOTIFY maxZChanged)
    Q_PROPERTY(float distance READ distance NOTIFY positionChanged)
    Q_PROPERTY(float azimuth READ azimuth NOTIFY positionChanged)
    Q_PROPERTY(float speed READ speed NOTIFY velocityChanged)
    Q_PROPERTY(float heading READ heading NOTIFY velocityChanged)
    Q_PROPERTY(float height READ height NOTIFY heightChanged)
    Q_PROPERTY(QString heightText READ heightText NOTIFY heightChanged)

   public:
    /// An item with no target yet (object id NonObjectPoint::PointNotYetAssigned).
    explicit ObjectItemViewModel(QObject* parent = nullptr);
    explicit ObjectItemViewModel(int objectId, QObject* parent = nullptr);

    /// The target's stable id; the same number as the "#n" label on the radar plot.
    [[nodiscard]] int objectId() const {
        return m_objectId;
    }
    [[nodiscard]] QString idText() const;

    /// False once the tracker has dropped the target (it lingers while its points fade).
    [[nodiscard]] bool isActive() const {
        return m_isActive;
    }
    void setIsActive(bool value);

    /// The tracker's own confidence in this track, as reported by the sensor.
    [[nodiscard]] float confidenceLevel() const {
        return m_confidenceLevel;
    }
    void setConfidenceLevel(float value);

    [[nodiscard]] MMWave::Vector3 position() const {
        return m_position;
    }
    void setPosition(MMWave::Vector3 value);

    [[nodiscard]] MMWave::Vector3 velocity() const {
        return m_velocity;
    }
    void setVelocity(MMWave::Vector3 value);

    [[nodiscard]] MMWave::Vector3 acceleration() const {
        return m_acceleration;
    }
    void setAcceleration(MMWave::Vector3 value);

    /// Lowest point of the target, metres.
    [[nodiscard]] float minZ() const {
        return m_minZ;
    }
    void setMinZ(float value);

    /// Highest point of the target, metres.
    [[nodiscard]] float maxZ() const {
        return m_maxZ;
    }
    void setMaxZ(float value);

    // ---- derived ----------------------------------------------------------------------------

    /// confidenceLevel (taken as 0..1) as a whole percentage, decimals cut off.
    [[nodiscard]] int confidencePercent() const {
        return static_cast<int>(m_confidenceLevel * 100);
    }
    [[nodiscard]] QString confidenceText() const;

    /// Metres from the sensor along the floor: what the radar grid's range arcs measure.
    [[nodiscard]] float distance() const;

    /// Degrees off boresight, negative = left: what the radar grid's rays measure.
    [[nodiscard]] float azimuth() const;

    /// Metres per second, in any direction.
    [[nodiscard]] float speed() const {
        return m_velocity.length();
    }

    /// Direction of travel in degrees: 0 = straight away from the sensor, positive = right.
    [[nodiscard]] float heading() const;

    /// Extent of the target from minZ to maxZ, metres.
    [[nodiscard]] float height() const {
        return m_maxZ - m_minZ;
    }

    [[nodiscard]] QString heightText() const;
    [[nodiscard]] QString minZText() const;
    [[nodiscard]] QString maxZText() const;

   signals:
    void isActiveChanged();
    void confidenceLevelChanged();
    void positionChanged();
    void velocityChanged();
    void accelerationChanged();
    void minZChanged();
    void maxZChanged();
    void heightChanged();

   private:
    static QVector3D toQt(MMWave::Vector3 v) {
        return {v.x, v.y, v.z};
    }
    QVector3D positionQt() const {
        return toQt(m_position);
    }
    QVector3D velocityQt() const {
        return toQt(m_velocity);
    }
    QVector3D accelerationQt() const {
        return toQt(m_acceleration);
    }

    int m_objectId;
    bool m_isActive = true;
    float m_confidenceLevel = 0;
    MMWave::Vector3 m_position;
    MMWave::Vector3 m_velocity;
    MMWave::Vector3 m_acceleration;
    float m_minZ = 0;
    float m_maxZ = 0;
};

}  // namespace Sarwave::Core::Gui::ViewModels::Object
