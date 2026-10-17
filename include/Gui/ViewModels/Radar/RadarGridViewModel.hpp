#pragma once

#include <QString>

#include "Gui/Models/RadarData.hpp"
#include "Gui/ViewModels/Radar/RadarSnapshot.hpp"
#include "Gui/ViewModels/ViewModelBase.hpp"

namespace Sarwave::Core::Gui::ViewModels::Radar {

/// Configuration of the radar grid: how much range/azimuth it shows and how it is subdivided.
class RadarGridViewModel : public ViewModelBase {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by the dashboard view models.")

    Q_PROPERTY(Sarwave::Core::Gui::ViewModels::Radar::RadarSnapshotPtr snapshot READ snapshot NOTIFY
                   snapshotChanged)
    Q_PROPERTY(double minRange READ minRange WRITE setMinRange NOTIFY minRangeChanged)
    Q_PROPERTY(double maxRange READ maxRange WRITE setMaxRange NOTIFY maxRangeChanged)
    Q_PROPERTY(double rangeStep READ rangeStep WRITE setRangeStep NOTIFY rangeStepChanged)
    Q_PROPERTY(double minAzimuth READ minAzimuth WRITE setMinAzimuth NOTIFY minAzimuthChanged)
    Q_PROPERTY(double maxAzimuth READ maxAzimuth WRITE setMaxAzimuth NOTIFY maxAzimuthChanged)
    Q_PROPERTY(double azimuthStep READ azimuthStep WRITE setAzimuthStep NOTIFY azimuthStepChanged)
    Q_PROPERTY(QString sensorLabel READ sensorLabel WRITE setSensorLabel NOTIFY sensorLabelChanged)

   public:
    /// No data source: shows nothing.
    explicit RadarGridViewModel(QObject* parent = nullptr);

    /// No data source. As in the previewer, shows a few sample targets and points.
    explicit RadarGridViewModel(DesignModeTag, QObject* parent = nullptr);

    explicit RadarGridViewModel(Models::RadarData& data, QObject* parent = nullptr);

    /// What the grid currently draws. Replaced as a whole on every frame.
    [[nodiscard]] RadarSnapshotPtr snapshot() const {
        return m_snapshot;
    }

    /// Inner radius in metres (blind zone). 0 = the sector starts at the sensor.
    [[nodiscard]] double minRange() const {
        return m_minRange;
    }
    void setMinRange(double value);

    /// Outer radius in metres.
    [[nodiscard]] double maxRange() const {
        return m_maxRange;
    }
    void setMaxRange(double value);

    /// Metres between range arcs.
    [[nodiscard]] double rangeStep() const {
        return m_rangeStep;
    }
    void setRangeStep(double value);

    /// Left edge of the field of view in degrees; 0 = boresight, negative = left.
    [[nodiscard]] double minAzimuth() const {
        return m_minAzimuth;
    }
    void setMinAzimuth(double value);

    /// Right edge of the field of view in degrees; positive = right.
    [[nodiscard]] double maxAzimuth() const {
        return m_maxAzimuth;
    }
    void setMaxAzimuth(double value);

    /// Degrees between radial guide-lines.
    [[nodiscard]] double azimuthStep() const {
        return m_azimuthStep;
    }
    void setAzimuthStep(double value);

    [[nodiscard]] QString sensorLabel() const {
        return m_sensorLabel;
    }
    void setSensorLabel(const QString& value);

    /// The copy the grid draws, taken from `data` on the thread that called RadarData::update.
    /// (Private in C#; public here so it can be tested.)
    static RadarSnapshot createSnapshot(const Models::RadarData& data);

    /// The previewer's sample snapshot: three targets and five fading frames scattered with
    /// System.Random(1).
    static RadarSnapshot designSnapshot();

   signals:
    void snapshotChanged();
    void minRangeChanged();
    void maxRangeChanged();
    void rangeStepChanged();
    void minAzimuthChanged();
    void maxAzimuthChanged();
    void azimuthStepChanged();
    void sensorLabelChanged();

   private:
    void setSnapshot(RadarSnapshotPtr value);

    RadarSnapshotPtr m_snapshot = RadarSnapshot::empty();
    double m_minRange = 0;
    double m_maxRange = 12;
    double m_rangeStep = 1;
    double m_minAzimuth = -45;
    double m_maxAzimuth = 45;
    double m_azimuthStep = 15;
    QString m_sensorLabel = QStringLiteral("IWR6843");

    LatestOnUiThread<RadarSnapshotPtr> m_snapshotPost{
        this, [this](RadarSnapshotPtr s) { setSnapshot(s); }};
    RadarDataSubscription m_subscription;  // last: ended first
};

}  // namespace Sarwave::Core::Gui::ViewModels::Radar
