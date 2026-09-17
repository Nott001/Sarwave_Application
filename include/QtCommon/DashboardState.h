#pragma once

#include <QObject>
#include <QString>

class DashboardState final : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool presenceDetected READ presenceDetected NOTIFY detectionsChanged)
    Q_PROPERTY(double centroidX READ centroidX NOTIFY detectionsChanged)
    Q_PROPERTY(double centroidY READ centroidY NOTIFY detectionsChanged)
    Q_PROPERTY(double centroidZ READ centroidZ NOTIFY detectionsChanged)
    Q_PROPERTY(double distance READ distance NOTIFY detectionsChanged)
    Q_PROPERTY(double dopplerVelocity READ dopplerVelocity NOTIFY detectionsChanged)
    Q_PROPERTY(double pointDensity READ pointDensity NOTIFY detectionsChanged)
    Q_PROPERTY(double snr READ snr NOTIFY detectionsChanged)
    Q_PROPERTY(double spatialSpread READ spatialSpread NOTIFY detectionsChanged)
    Q_PROPERTY(double classificationConfidence READ classificationConfidence NOTIFY detectionsChanged)
    Q_PROPERTY(QString lastUpdated READ lastUpdated NOTIFY detectionsChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(bool demoMode READ demoMode CONSTANT)

public:
    explicit DashboardState(QObject *parent = nullptr);

    bool presenceDetected() const;
    double centroidX() const;
    double centroidY() const;
    double centroidZ() const;
    double distance() const;
    double dopplerVelocity() const;
    double pointDensity() const;
    double snr() const;
    double spatialSpread() const;
    double classificationConfidence() const;
    QString lastUpdated() const;
    QString connectionStatus() const;
    bool demoMode() const;

signals:
    void detectionsChanged();
    void connectionStatusChanged();

private:
    bool m_presenceDetected = false;
    double m_centroidX = 0.0;
    double m_centroidY = 0.0;
    double m_centroidZ = 0.0;
    double m_distance = 0.0;
    double m_dopplerVelocity = 0.0;
    double m_pointDensity = 0.0;
    double m_snr = 0.0;
    double m_spatialSpread = 0.0;
    double m_classificationConfidence = 0.0;
    QString m_lastUpdated;
};
