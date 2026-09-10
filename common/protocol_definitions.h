#pragma once
#include <QString>

struct SensorData {
    double temperature = 0.0;
    double humidity = 0.0;
    double pressure = 0.0;
    QString timestamp;
};

SensorData parseSensorData(const QString& json);
QString serializeSensorData(const SensorData& data);
