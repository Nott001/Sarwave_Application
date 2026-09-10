#include "protocol_definitions.h"
#include <QJsonDocument>
#include <QJsonObject>

SensorData parseSensorData(const QString& json) {
    SensorData data;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        data.temperature = obj["temperature"].toDouble(0.0);
        data.humidity = obj["humidity"].toDouble(0.0);
        data.pressure = obj["pressure"].toDouble(0.0);
        data.timestamp = obj["timestamp"].toString();
    }
    return data;
}

QString serializeSensorData(const SensorData& data) {
    QJsonObject obj;
    obj["temperature"] = data.temperature;
    obj["humidity"] = data.humidity;
    obj["pressure"] = data.pressure;
    obj["timestamp"] = data.timestamp;
    return QJsonDocument(obj).toJson();
}
