#include <QCoreApplication>
#include <QtTest>
#include "DashboardState.h"
#include "ProtocolDefinitions.h"
#include "MMWave/Prompting/General.hpp"
#include "MMWave/Tlv.hpp"
#include "MMWave/Streaming.hpp"
#include <vector>
#include <cstdint>
#include <cstring>
#include <iostream>

class DashboardStateTest : public QObject {
    Q_OBJECT
private slots:
    void testDefaultValues() {
        DashboardState state;
        QVERIFY(!state.presenceDetected());
        QVERIFY(state.pointCloud().isEmpty());
        QCOMPARE(state.centroidX(), 0.0);
        QCOMPARE(state.centroidY(), 0.0);
        QCOMPARE(state.centroidZ(), 0.0);
        QCOMPARE(state.distance(), 0.0);
        QCOMPARE(state.dopplerVelocity(), 0.0);
        QCOMPARE(state.pointDensity(), 0.0);
        QCOMPARE(state.snr(), 0.0);
        QCOMPARE(state.spatialSpread(), 0.0);
        QCOMPARE(state.classificationConfidence(), 0.0);
        QCOMPARE(state.demoMode(), true);
    }

    void testUpdatePointCloudSetsPresence() {
        DashboardState state;
        QVariantList points;
        QVariantMap p;
        p.insert("x", 1.0); p.insert("y", 2.0);
        p.insert("z", 0.5); p.insert("v", 0.3);
        points.append(p);
        state.updatePointCloud(points);
        QVERIFY(state.presenceDetected());
        QCOMPARE(state.pointCloud().size(), 1);
    }

    void testEmptyPointCloudClearsPresence() {
        DashboardState state;
        QVariantList points;
        QVariantMap p;
        p.insert("x", 1.0); p.insert("y", 2.0);
        points.append(p);
        state.updatePointCloud(points);
        QVERIFY(state.presenceDetected());
        QVariantList empty;
        state.updatePointCloud(empty);
        QVERIFY(!state.presenceDetected());
    }

    void testConnectionStatusIsString() {
        DashboardState state;
        QVERIFY(!state.connectionStatus().isEmpty());
    }
};

class ProtocolDefinitionsTest : public QObject {
    Q_OBJECT
private slots:
    void testParseRadarData_validJson() {
        QString json = R"({"x":1.5,"y":2.3,"z":0.8,"dopplerVelocity":0.5,"pointDensity":100,"snr":12.5,"spatialSpread":3.2,"distance":5.0,"classificationConfidence":0.95,"presenceDetected":true,"timestamp":"2026-01-01T00:00:00"})";
        RadarDetection data = parseRadarData(json);
        QCOMPARE(data.x, 1.5); QCOMPARE(data.y, 2.3); QCOMPARE(data.z, 0.8);
        QCOMPARE(data.dopplerVelocity, 0.5); QCOMPARE(data.pointDensity, 100.0);
        QCOMPARE(data.snr, 12.5); QCOMPARE(data.spatialSpread, 3.2);
        QCOMPARE(data.distance, 5.0); QCOMPARE(data.classificationConfidence, 0.95);
        QVERIFY(data.presenceDetected);
        QCOMPARE(data.timestamp, QString("2026-01-01T00:00:00"));
    }

    void testParseRadarData_missingFields_defaultsToZero() {
        QString json = R"({})";
        RadarDetection data = parseRadarData(json);
        QCOMPARE(data.x, 0.0); QCOMPARE(data.y, 0.0); QCOMPARE(data.z, 0.0);
        QVERIFY(!data.presenceDetected);
    }

    void testSerializeRoundTrip() {
        RadarDetection original;
        original.x = 1.5; original.y = 2.3; original.z = 0.8;
        original.dopplerVelocity = 0.5; original.pointDensity = 100.0;
        original.snr = 12.5; original.spatialSpread = 3.2;
        original.distance = 5.0; original.classificationConfidence = 0.95;
        original.presenceDetected = true;
        original.timestamp = "2026-01-01T00:00:00";
        QString json = serializeRadarData(original);
        RadarDetection parsed = parseRadarData(json);
        QCOMPARE(parsed.x, original.x); QCOMPARE(parsed.y, original.y); QCOMPARE(parsed.z, original.z);
        QCOMPARE(parsed.dopplerVelocity, original.dopplerVelocity);
        QCOMPARE(parsed.pointDensity, original.pointDensity); QCOMPARE(parsed.snr, original.snr);
        QCOMPARE(parsed.spatialSpread, original.spatialSpread); QCOMPARE(parsed.distance, original.distance);
        QCOMPARE(parsed.classificationConfidence, original.classificationConfidence);
        QCOMPARE(parsed.presenceDetected, original.presenceDetected);
        QCOMPARE(parsed.timestamp, original.timestamp);
    }
};

class GeneralTest : public QObject {
    Q_OBJECT
private slots:
    void testCheckStrArgument_validPositive() {
        QCOMPARE(MMWave::Prompting::General::checkStrArgument("5"), 5);
        QCOMPARE(MMWave::Prompting::General::checkStrArgument("0"), 0);
        QCOMPARE(MMWave::Prompting::General::checkStrArgument("999"), 999);
    }

    void testCheckStrArgument_negative_throws() {
        QVERIFY_EXCEPTION_THROWN(MMWave::Prompting::General::checkStrArgument("-1"), std::out_of_range);
    }

    void testCheckStrArgument_nonNumeric_throws() {
        QVERIFY_EXCEPTION_THROWN(MMWave::Prompting::General::checkStrArgument("abc"), std::invalid_argument);
    }

    void testBuildPortName_linux() {
#ifdef _WIN32
        QSKIP("Windows-specific test");
#else
        QCOMPARE(MMWave::Prompting::General::buildPortName(3), "/dev/ttyUSB3");
#endif
    }

    void testBuildPortName_windows() {
#ifdef _WIN32
        QCOMPARE(MMWave::Prompting::General::buildPortName(3), "COM3");
#else
        QSKIP("Linux-specific test");
#endif
    }

    void testCreatePath() {
        auto paths = MMWave::Prompting::General::createPath(1, 2);
#ifdef _WIN32
        QCOMPARE(paths.cli, "COM1"); QCOMPARE(paths.data, "COM2");
#else
        QCOMPARE(paths.cli, "/dev/ttyUSB1"); QCOMPARE(paths.data, "/dev/ttyUSB2");
#endif
    }
};

class TlvTest : public QObject {
    Q_OBJECT
private slots:
    void testStructSizes() {
        QCOMPARE(sizeof(MMWave::Streaming::MAGIC_WORD), 8u);
        QCOMPARE(sizeof(MMWave::Streaming::FrameHeader), 32u);
        QCOMPARE(sizeof(MMWave::Tlv::TlvHeader), 8u);
        QCOMPARE(sizeof(MMWave::Tlv::DetectedPoint), 16u);
    }

    void testMagicWord() {
        uint8_t buf[8];
        std::memcpy(buf, MMWave::Streaming::MAGIC_WORD, 8);
        QCOMPARE(std::memcmp(buf, MMWave::Streaming::MAGIC_WORD, 8), 0);
    }

    void testFrameHeaderPacked() {
        MMWave::Streaming::FrameHeader h{};
        h.version = 0x00010002;
        h.totalPacketLen = 100;
        h.platform = 3;
        h.frameNumber = 4;
        h.timeCpuCycles = 5;
        h.numDetectedObj = 6;
        h.numTLVs = 1;
        h.subFrameNumber = 7;
        uint8_t bytes[sizeof(h)];
        std::memcpy(bytes, &h, sizeof(h));
        MMWave::Streaming::FrameHeader h2;
        std::memcpy(&h2, bytes, sizeof(h2));
        QCOMPARE(h2.version, h.version);
        QCOMPARE(h2.totalPacketLen, h.totalPacketLen);
        QCOMPARE(h2.numDetectedObj, h.numDetectedObj);
        QCOMPARE(h2.numTLVs, h.numTLVs);
    }

    void testTlvParsing() {
        MMWave::Streaming::Frame sf;
        sf.header.numTLVs = 1;
        sf.header.numDetectedObj = 2;
        sf.header.totalPacketLen = 8 + 32 + 8 + 32;

        std::vector<uint8_t>& bytes = sf.bytes;
        bytes.insert(bytes.end(), std::begin(MMWave::Streaming::MAGIC_WORD), std::end(MMWave::Streaming::MAGIC_WORD));
        uint8_t hb[32];
        std::memcpy(hb, &sf.header, 32);
        bytes.insert(bytes.end(), hb, hb + 32);
        MMWave::Tlv::TlvHeader tlh{1, 32};
        uint8_t tlhb[8];
        std::memcpy(tlhb, &tlh, 8);
        bytes.insert(bytes.end(), tlhb, tlhb + 8);
        MMWave::Tlv::DetectedPoint p1{1.0f, 2.0f, 0.5f, 0.3f};
        bytes.insert(bytes.end(), reinterpret_cast<const uint8_t*>(&p1), reinterpret_cast<const uint8_t*>(&p1) + 16);
        MMWave::Tlv::DetectedPoint p2{3.0f, 4.0f, 1.0f, -0.5f};
        bytes.insert(bytes.end(), reinterpret_cast<const uint8_t*>(&p2), reinterpret_cast<const uint8_t*>(&p2) + 16);

        QCOMPARE(bytes.size(), 80u);
        const uint8_t* tlvOff = bytes.data() + 8 + 32;
        MMWave::Tlv::TlvHeader rl;
        std::memcpy(&rl, tlvOff, sizeof(rl));
        QCOMPARE(rl.type, MMWave::Tlv::TLV_DETECTED_POINTS);
        QCOMPARE(rl.length, 32u);
        const MMWave::Tlv::DetectedPoint* pts = reinterpret_cast<const MMWave::Tlv::DetectedPoint*>(tlvOff + 8);
        QCOMPARE(pts[0].x, 1.0f);
        QCOMPARE(pts[0].y, 2.0f);
        QCOMPARE(pts[1].x, 3.0f);
        QCOMPARE(pts[1].y, 4.0f);

        int count = 0;
        int idx = 0;
        for (const auto& tlv : MMWave::Tlv::TlvRange(sf)) {
            if (tlv.type == MMWave::Tlv::TLV_DETECTED_POINTS) {
                for (const auto& pt : tlv.points()) {
                    QCOMPARE(pt.x, idx == 0 ? 1.0f : 3.0f);
                    QCOMPARE(pt.y, idx == 0 ? 2.0f : 4.0f);
                    idx++;
                    count++;
                }
            }
        }
        QCOMPARE(count, 2);
    }

    void testTlvRange_outOfBounds_stopsEarly() {
        MMWave::Streaming::Frame sf;
        sf.header.numTLVs = 1;
        sf.header.totalPacketLen = 100;
        std::vector<uint8_t>& bytes = sf.bytes;
        bytes.insert(bytes.end(), std::begin(MMWave::Streaming::MAGIC_WORD), std::end(MMWave::Streaming::MAGIC_WORD));
        uint8_t hb[sizeof(MMWave::Streaming::FrameHeader)];
        std::memcpy(hb, &sf.header, sizeof(MMWave::Streaming::FrameHeader));
        bytes.insert(bytes.end(), hb, hb + sizeof(hb));
        bool found = false;
        for (const auto& tlv : MMWave::Tlv::TlvRange(sf)) {
            (void)tlv; found = true;
        }
        QVERIFY(!found);
    }
};

class StreamingTest : public QObject {
    Q_OBJECT
private slots:
void testMagicWordMatches() {
        uint8_t buf[8];
        std::memcpy(buf, MMWave::Streaming::MAGIC_WORD, 8);
        QCOMPARE(std::memcmp(buf, MMWave::Streaming::MAGIC_WORD, 8), 0);
    }

    void testMagicWordIs8Bytes() {
        QCOMPARE(sizeof(MMWave::Streaming::MAGIC_WORD), 8u);
    }

    void testFrameHeaderSizeIsPacked() {
        QCOMPARE(sizeof(MMWave::Streaming::FrameHeader), 32u);
    }
};

int main(int argc, char *argv[]) {
    int failed = 0;
    failed |= QTest::qExec(new DashboardStateTest, argc, argv);
    failed |= QTest::qExec(new ProtocolDefinitionsTest, 0, nullptr);
    failed |= QTest::qExec(new GeneralTest, 0, nullptr);
    failed |= QTest::qExec(new TlvTest, 0, nullptr);
    failed |= QTest::qExec(new StreamingTest, 0, nullptr);
    return failed;
}
#include "test_main.moc"
