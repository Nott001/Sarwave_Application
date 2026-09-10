#pragma once

#include <QObject>
#include <QString>

class QTimer;

// Presentation state shared by both applications. The mock refresh can later be
// replaced by updates from the networking layer without changing QML bindings.
class DashboardState final : public QObject {
    Q_OBJECT

    Q_PROPERTY(double temperature READ temperature NOTIFY readingsChanged)
    Q_PROPERTY(double humidity READ humidity NOTIFY readingsChanged)
    Q_PROPERTY(double pressure READ pressure NOTIFY readingsChanged)
    Q_PROPERTY(QString lastUpdated READ lastUpdated NOTIFY readingsChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(bool demoMode READ demoMode CONSTANT)

public:
    explicit DashboardState(QObject *parent = nullptr);

    double temperature() const;
    double humidity() const;
    double pressure() const;
    QString lastUpdated() const;
    QString connectionStatus() const;
    bool demoMode() const;

    Q_INVOKABLE void refresh();

signals:
    void readingsChanged();
    void connectionStatusChanged();

private:
    void updateDemoReadings();

    QTimer *m_refreshTimer;
    double m_temperature = 24.8;
    double m_humidity = 58.4;
    double m_pressure = 1012.6;
    QString m_lastUpdated;
    int m_sample = 0;
};
