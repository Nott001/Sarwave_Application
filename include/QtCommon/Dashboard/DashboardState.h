#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class DashboardState final : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool presenceDetected READ presenceDetected NOTIFY detectionsChanged)
    Q_PROPERTY(QVariantList pointCloud READ pointCloud NOTIFY pointCloudChanged)
    Q_PROPERTY(double centroidX READ centroidX NOTIFY detectionsChanged)
    Q_PROPERTY(double centroidY READ centroidY NOTIFY detectionsChanged)
    Q_PROPERTY(double centroidZ READ centroidZ NOTIFY detectionsChanged)
    Q_PROPERTY(double distance READ distance NOTIFY detectionsChanged)
    Q_PROPERTY(double dopplerVelocity READ dopplerVelocity NOTIFY detectionsChanged)
    Q_PROPERTY(double pointDensity READ pointDensity NOTIFY detectionsChanged)
    Q_PROPERTY(double snr READ snr NOTIFY detectionsChanged)
    Q_PROPERTY(double spatialSpread READ spatialSpread NOTIFY detectionsChanged)
    Q_PROPERTY(
        double classificationConfidence READ classificationConfidence NOTIFY detectionsChanged)
    Q_PROPERTY(QString lastUpdated READ lastUpdated NOTIFY detectionsChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(bool demoMode READ demoMode CONSTANT)
    Q_PROPERTY(double maxRangeMetres READ maxRangeMetres CONSTANT)
    Q_PROPERTY(QVariantList drawings READ drawings NOTIFY drawingsChanged)

   public:
    explicit DashboardState(double maxRangeMetres = 5.0, QObject* parent = nullptr);

    [[nodiscard]] double maxRangeMetres() const;

    [[nodiscard]] bool presenceDetected() const;
    [[nodiscard]] QVariantList pointCloud() const;
    [[nodiscard]] double centroidX() const;
    [[nodiscard]] double centroidY() const;
    [[nodiscard]] double centroidZ() const;
    [[nodiscard]] double distance() const;
    [[nodiscard]] double dopplerVelocity() const;
    [[nodiscard]] double pointDensity() const;
    [[nodiscard]] double snr() const;
    [[nodiscard]] double spatialSpread() const;
    [[nodiscard]] double classificationConfidence() const;
    [[nodiscard]] QString lastUpdated() const;
    [[nodiscard]] QString connectionStatus() const;
    [[nodiscard]] bool demoMode() const;
    [[nodiscard]] QVariantList drawings() const;

    // Replaces the point cloud with `points` (a list of maps with x, y, z and
    // v fields) and marks presence accordingly.
    void updatePointCloud(const QVariantList& points);

    void beginDrawings();
    void endDrawings();
    void addLine(double x1, double y1, double x2, double y2, double thickness = 2.0,
                 const QString& colour = QStringLiteral("#FF0000"));
    void addCircle(double cx, double cy, double radius, double thickness = 2.0,
                   const QString& colour = QStringLiteral("#FF0000"),
                   const QString& fillColour = QString());
    void clearDrawings();

   signals:
    void detectionsChanged();
    void connectionStatusChanged();
    void pointCloudChanged();
    void drawingsChanged();

   private:
    bool m_presenceDetected = false;
    QVariantList m_pointCloud;
    QVariantList m_drawings;
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
    double m_maxRangeMetres = 5.0;
    bool m_batchDrawing = false;
};
