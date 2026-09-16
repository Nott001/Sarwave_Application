#pragma once
#include <QString>

struct RadarDetection {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double dopplerVelocity = 0.0;
    double pointDensity = 0.0;
    double snr = 0.0;
    double spatialSpread = 0.0;
    double distance = 0.0;
    double classificationConfidence = 0.0;
    bool presenceDetected = false;
    QString timestamp;
};

RadarDetection parseRadarData(const QString& json);
QString serializeRadarData(const RadarDetection& data);
