#include "ProtocolDefinitions.h"

#include <QJsonDocument>
#include <QJsonObject>

RadarDetection parseRadarData(const QString& json)
{
    RadarDetection data;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        data.x = obj["x"].toDouble(0.0);
        data.y = obj["y"].toDouble(0.0);
        data.z = obj["z"].toDouble(0.0);
        data.dopplerVelocity = obj["dopplerVelocity"].toDouble(0.0);
        data.pointDensity = obj["pointDensity"].toDouble(0.0);
        data.snr = obj["snr"].toDouble(0.0);
        data.spatialSpread = obj["spatialSpread"].toDouble(0.0);
        data.distance = obj["distance"].toDouble(0.0);
        data.classificationConfidence = obj["classificationConfidence"].toDouble(0.0);
        data.presenceDetected = obj["presenceDetected"].toBool(false);
        data.timestamp = obj["timestamp"].toString();
    }
    return data;
}

QString serializeRadarData(const RadarDetection& data)
{
    QJsonObject obj;
    obj["x"] = data.x;
    obj["y"] = data.y;
    obj["z"] = data.z;
    obj["dopplerVelocity"] = data.dopplerVelocity;
    obj["pointDensity"] = data.pointDensity;
    obj["snr"] = data.snr;
    obj["spatialSpread"] = data.spatialSpread;
    obj["distance"] = data.distance;
    obj["classificationConfidence"] = data.classificationConfidence;
    obj["presenceDetected"] = data.presenceDetected;
    obj["timestamp"] = data.timestamp;
    return QJsonDocument(obj).toJson();
}
