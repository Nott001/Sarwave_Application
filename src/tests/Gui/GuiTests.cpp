#include <QCoreApplication>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <memory>
#include <numbers>
#include <thread>

#include "../Core/FrameBuilder.hpp"
#include "DevEnv/ConfigMemory.hpp"
#include "DevEnv/DevEnv.hpp"
#include "DevEnv/Gui/CfgDialogue.hpp"
#include "DevEnv/Gui/PortDialogue.hpp"
#include "DevEnv/Gui/Run.hpp"
#include "Gui/Common/DotNetRandom.hpp"
#include "Gui/Models/RadarData.hpp"
#include "Gui/ViewModels/ImuStatusViewModel.hpp"
#include "Gui/ViewModels/MinimalGuiViewModel.hpp"
#include "Gui/ViewModels/Object/ObjectsPanelViewModel.hpp"
#include "Gui/ViewModels/Radar/RadarGridViewModel.hpp"
#include "Gui/ViewModels/StatusViewModel.hpp"
#include "Gui/Views/GuiHost.hpp"
#include "Gui/Views/Radar/ColourManager.hpp"
#include "Gui/Views/Radar/RadarDataOverlay.hpp"
#include "Gui/Views/Radar/RadarGeometry.hpp"
#include "Gui/Views/StringFormat.hpp"
#include "Integration/PeopleTracking/ImuFrames.hpp"
#include "MMWave/Exceptions/FrameMalformedException.hpp"
#include "TestFramework.hpp"

using namespace Sarwave::Core;
using namespace Sarwave::Core::Gui;
using SarwaveTest::FrameBuilder;

namespace {

QGuiApplication& app() {
    static int argc = 1;
    static char name[] = "GuiTests";
    static char* argv[] = {name, nullptr};
    // Deliberately never destroyed: tearing a QGuiApplication down from an exit handler runs
    // after Qt's own statics are gone.
    static QGuiApplication* instance = [] {
        qputenv("QT_QPA_PLATFORM", "offscreen");
        auto* a = new QGuiApplication(argc, argv);
        Views::initialize();
        Sarwave::DevEnv::initialize();
        return a;
    }();
    return *instance;
}

void processEvents(int ms = 50) {
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < ms) QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
}

bool near(double a, double b) {
    return std::abs(a - b) < 1e-9;
}

}  // namespace

TEST_CASE("RadarGeometry fits the sector like the C# version") {
    using Views::Radar::RadarGeometry;
    const auto g = RadarGeometry::create(QRectF(34, 20, 700, 400), 0, 12, -45, 45);
    // Bounding box 2*12*sin45 = 16.97 m wide, 12 m tall: height limits the scale.
    CHECK(near(g.scale, 400.0 / 12.0));
    CHECK(near(g.origin.y(), 420.0));
    CHECK(near(g.origin.x(), 34 + 350.0));
    CHECK(g.contains(0, 5));
    CHECK(!g.contains(5, 1));     // outside the +-45 degree fan
    CHECK(!g.contains(0, 12.5));  // beyond max range
    const QPointF top = g.fromPolar(0, 12);
    CHECK(near(top.x(), g.origin.x()) && near(top.y(), 20.0));
    CHECK(RadarGeometry::create(QRectF(0, 0, 0, 10), 0, 12, -45, 45).isEmpty());
}

TEST_CASE("ColourManager hands out palette colours and point colours") {
    auto& colours = Views::Radar::ColourManager::shared();
    CHECK(colours.forTarget(0) == QColor("#3dff6d"));
    CHECK(colours.forTarget(9) == colours.forTarget(1));     // wraps after 8
    CHECK(colours.fromPalette(-1) == QColor("#3dffdf"));     // wraps backwards too
    CHECK(colours.forPoints(-3) == QColor("#4a5a75"));       // rejected: noise
    CHECK(colours.forPoints(INT_MIN) == QColor("#91a4c2"));  // not yet assigned
}

TEST_CASE("DotNetRandom reproduces System.Random(seed)") {
    // Reference values printed by .NET 10 (see Parity/csharp, command "random").
    Common::DotNetRandom r(1);
    const float expected[] = {0.24866858f, 0.11074398f, 0.46701068f,
                              0.7716041f,  0.6575189f,  0.4327826f};
    for (float e : expected) CHECK_EQ(r.nextSingle(), e);
    Common::DotNetRandom r42(42);
    CHECK_EQ(r42.next(), 1434747710);
    CHECK_EQ(r42.next(), 302596119);
    CHECK_EQ(r42.nextDouble(), 0.12551828945312568);
}

TEST_CASE("StringFormat renders like the AXAML StringFormat bindings") {
    CHECK(Views::StringFormat::fixed(2.5, 0, 1) == "2.5");
    CHECK(Views::StringFormat::fixed(0, 0, 1) == "0");
    CHECK(Views::StringFormat::fixed(12, 0, 2) == "12");
}

TEST_CASE("ObjectItemViewModel derives its figures like the C# one") {
    app();
    ViewModels::Object::ObjectItemViewModel item(3);
    item.setConfidenceLevel(0.874f);
    item.setMinZ(0.12f);
    item.setMaxZ(1.68f);
    item.setPosition({3, 4, 1});
    item.setVelocity({1, 1, 0});
    CHECK_EQ(item.confidencePercent(), 87);
    CHECK(item.confidenceText() == "87%");
    CHECK(item.idText() == "#3");
    CHECK(item.heightText() == "1.56");
    CHECK(item.distance() == 5.0f);
    CHECK(std::abs(item.heading() - 45.0f) < 1e-4f);
}

TEST_CASE(
    "ObjectsPanelViewModel adds in first-appearance order, updates in place, removes gone ones") {
    app();
    using Panel = ViewModels::Object::ObjectsPanelViewModel;
    Panel panel;
    const auto values = [](int id, bool active) {
        return Panel::TargetValues{id, active, 0.5f, {}, {}, {}, 0, 1};
    };

    panel.apply({values(4, true), values(2, true)});
    REQUIRE(panel.count() == 2);
    auto* four = panel.tryGet(4);
    CHECK(panel.items()[0] == four);

    panel.apply({values(2, true), values(4, false), values(7, true)});
    CHECK(panel.tryGet(4) == four);  // the same object, updated
    CHECK(!four->isActive());
    CHECK_EQ(panel.items()[2]->objectId(), 7);

    panel.apply({values(7, true)});
    CHECK_EQ(panel.count(), 1);
    CHECK(!panel.contains(4));
    CHECK(panel.rowCount() == 1);
}

TEST_CASE("RadarGridViewModel snapshots targets, groups and fade from RadarData") {
    app();
    Models::RadarData data(MMWave::TimeSpans::fromSeconds(1));
    data.update(FrameBuilder().targets({1}).heights({1}, 1.7f, 0.2f).cloud(3).frame(0),
                MMWave::TimeSpan(0));
    data.update(
        FrameBuilder().targets({1}).heights({1}, 1.7f, 0.2f).indexes({1, 253, 1}).cloud(2).frame(1),
        MMWave::TimeSpan(2'500'000));

    const auto snapshot = ViewModels::Radar::RadarGridViewModel::createSnapshot(data);
    REQUIRE(snapshot.targets().size() == 1);
    CHECK(snapshot.targets()[0].isActive);
    REQUIRE(snapshot.pointClouds().size() == 2);

    const auto& older = snapshot.pointClouds()[0];
    CHECK(std::abs(older.fade - 0.25f) < 1e-6f);  // 250 ms of 1 s
    REQUIRE(older.groups.size() == 2);            // rejected first, then the target
    CHECK_EQ(older.groups[0].objectId, -1);
    CHECK_EQ(older.groups[1].points.size(), std::size_t{2});

    const auto& newer = snapshot.pointClouds()[1];
    CHECK(newer.fade == 0.0f);
    REQUIRE(newer.groups.size() == 1);
    CHECK(newer.groups[0].isUnassigned());
}

TEST_CASE("Live view models follow RadarData across threads") {
    app();
    Models::RadarData data(MMWave::TimeSpans::fromSeconds(1));
    auto panel = std::make_unique<ViewModels::Object::ObjectsPanelViewModel>(data);
    std::thread worker([&] {
        data.update(
            FrameBuilder().targets({5, 6}).heights({5, 6}, 1.7f, 0.2f).presence(true).frame(0),
            MMWave::TimeSpan(0));
    });
    worker.join();
    processEvents();
    CHECK_EQ(panel->count(), 2);
    panel.reset();  // unsubscribes; later updates must not touch it
    data.update(FrameBuilder().targets({5}).heights({5}, 1.7f, 0.2f).frame(1),
                MMWave::TimeSpan(100));
    processEvents();
}

TEST_CASE("A GUI that falls behind is handed only the newest state") {
    app();
    Models::RadarData data(MMWave::TimeSpans::fromSeconds(1));
    ViewModels::Radar::RadarGridViewModel radar(data);
    ViewModels::Object::ObjectsPanelViewModel panel(data);
    int snapshots = 0;
    QObject::connect(&radar, &ViewModels::Radar::RadarGridViewModel::snapshotChanged,
                     [&] { snapshots++; });

    // 30 frames arrive while the UI thread is busy (here: blocked in join).
    std::thread worker([&] {
        for (std::uint32_t n = 0; n < 30; n++) {
            const std::vector<std::uint8_t> ids =
                n < 29 ? std::vector<std::uint8_t>{1, 2, 3} : std::vector<std::uint8_t>{7};
            data.update(FrameBuilder().targets(ids).heights(ids, 1.7f, 0.2f).frame(n),
                        MMWave::TimeSpan(n * 550'000));
        }
    });
    worker.join();
    processEvents();

    // One delivery, of the last frame: 1, 2 and 3 are gone (no points kept them), 7 is new.
    CHECK_EQ(snapshots, 1);
    REQUIRE(radar.snapshot()->targets().size() == 1);
    CHECK_EQ(radar.snapshot()->targets()[0].objectId, 3);  // ids follow arrival: 7 is object 3
    CHECK_EQ(panel.count(), 1);
    CHECK(panel.tryGet(3) != nullptr);
}

TEST_CASE("RadarDataOverlay's point sprites look like the ellipses they replace") {
    app();
    using namespace ViewModels::Radar;

    // A busy frame's worth of clouds: 18 clouds fading out, 8 targets and some rejected points.
    Common::DotNetRandom random(3);
    std::vector<PointCloudSnapshot> clouds;
    for (int c = 0; c < 18; c++) {
        std::vector<PointGroupSnapshot> groups;
        for (int id = -3; id < 8; id++) {
            const MMWave::Vector3 centre{(random.nextSingle() - 0.5f) * 8,
                                         random.nextSingle() * 10 + 0.5f, 0};
            std::vector<MMWave::Vector3> points;
            for (int k = 0; k < 14; k++) {
                points.push_back(centre + MMWave::Vector3{(random.nextSingle() - 0.5f) * 0.6f,
                                                          (random.nextSingle() - 0.5f) * 0.6f, 0});
            }
            groups.push_back({id, std::move(points)});
        }
        clouds.push_back({1.0f - static_cast<float>(c + 1) / 18, std::move(groups)});
    }
    const auto snapshot =
        std::make_shared<const RadarSnapshot>(std::vector<TargetSnapshot>{}, std::move(clouds));

    Views::Radar::RadarDataOverlay overlay;
    Views::Radar::RadarGeometry geometry;
    geometry.maxRange = 12;
    geometry.origin = QPointF(400, 470);
    geometry.scale = 38;
    overlay.setProperty("geometry", QVariant::fromValue(geometry));
    overlay.setProperty("snapshot", QVariant::fromValue(snapshot));

    const auto render = [&](const QTransform& transform) {
        QImage image(800, 480, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        painter.setWorldTransform(transform);
        overlay.paint(&painter);
        return image;
    };
    const QImage sprites = render(QTransform());
    // A not quite even scale takes the ellipse path, at what is the same size to the eye.
    const QImage ellipses = render(QTransform::fromScale(1 + 1e-9, 1));

    int worst = 0;
    double total = 0;
    int drawn = 0;
    for (int y = 0; y < sprites.height(); y++) {
        const auto* a = reinterpret_cast<const QRgb*>(sprites.constScanLine(y));
        const auto* b = reinterpret_cast<const QRgb*>(ellipses.constScanLine(y));
        for (int x = 0; x < sprites.width(); x++) {
            const int d = std::max(
                {std::abs(qRed(a[x]) - qRed(b[x])), std::abs(qGreen(a[x]) - qGreen(b[x])),
                 std::abs(qBlue(a[x]) - qBlue(b[x])), std::abs(qAlpha(a[x]) - qAlpha(b[x]))});
            worst = std::max(worst, d);
            total += d;
            if (qAlpha(b[x]) != 0) drawn++;
        }
    }

    REQUIRE(drawn > 5000);
    CHECK(worst <= 40);          // of 255, at the edge of a dot moved by 1/16 pixel
    CHECK(total / drawn < 4.0);  // on average, invisible
}

TEST_CASE("The dashboards load into a window and replace each other") {
    app();
    // The data outlives the dashboards that follow it, as in the apps.
    Models::RadarData data(MMWave::TimeSpans::fromSeconds(1));
    QQmlEngine engine;
    QQuickWindow window;
    window.resize(900, 450);
    window.show();

    QQuickItem* first = Views::showMaximalGui(window, engine, data);
    REQUIRE(first != nullptr);
    data.update(
        FrameBuilder().targets({1}).heights({1}, 1.7f, 0.2f).cloud(4).presence(true).frame(),
        MMWave::TimeSpan(0));
    processEvents();
    QQuickItem* second = Views::showMinimalGui(window, engine, data);
    REQUIRE(second != nullptr);
    processEvents();
    CHECK_EQ(window.contentItem()->childItems().size(), qsizetype{1});
    CHECK(second->width() == 900);
}

TEST_CASE("Port and cfg dialogues load, and cancelling returns nothing") {
    app();
    QQmlEngine engine;
    QQuickWindow owner;
    owner.resize(800, 450);

    // Cancel each dialogue as soon as it is up, through its own Cancel logic.
    const auto cancelSoon = [] {
        QTimer::singleShot(200, [] {
            for (QWindow* w : QGuiApplication::topLevelWindows()) {
                QVariant controller = w->property("controller");
                if (auto* port = qobject_cast<Sarwave::DevEnv::Gui::PortDialogue*>(
                        controller.value<QObject*>())) {
                    CHECK(port->defaultPortDirectory() == "/dev/ttyUSB");
                    port->cancel();
                }
                if (auto* cfg = qobject_cast<Sarwave::DevEnv::Gui::CfgDialogue*>(
                        controller.value<QObject*>())) {
                    CHECK(cfg->initialPath() == "/x/y.cfg");
                    cfg->apply("");  // shows the validation error, stays open
                    CHECK(cfg->errorText() == "Please select a configuration file.");
                    cfg->cancel();
                }
            }
        });
    };

    cancelSoon();
    CHECK(Sarwave::DevEnv::Gui::PortDialogue::showAsync(&owner, engine) == nullptr);
    cancelSoon();
    CHECK(!Sarwave::DevEnv::Gui::CfgDialogue::showAsync(&owner, engine, nullptr, "/x/y.cfg")
               .has_value());
}

TEST_CASE("ConfigMemory writes config.json the way System.Text.Json does") {
    using Sarwave::DevEnv::ConfigMemory;
#ifndef _WIN32
    CHECK_EQ(ConfigMemory::serialize("/home/u/AOP_6m_default.cfg"),
             "{\n  \"CfgPath\": \"/home/u/AOP_6m_default.cfg\"\n}");
    CHECK_EQ(ConfigMemory::serialize("a\"b\\c+<é>"),
             "{\n  \"CfgPath\": \"a\\u0022b\\\\c\\u002B\\u003C\\u00E9\\u003E\"\n}");
#endif
}

TEST_CASE("Status shows HOLD STEADY over everything else while gating refuses frames") {
    app();
    ViewModels::StatusViewModel status;
    CHECK(status.statusText() == QStringLiteral("SCANNING…"));
    status.setPresenceDetected(true);
    CHECK(status.statusText() == QStringLiteral("TARGET ACQUIRED"));
    status.setHoldSteady(true);
    CHECK(status.statusText() == QStringLiteral("HOLD STEADY"));
    CHECK(status.badgeText() == QStringLiteral("DETECTED"));
    status.setHoldSteady(false);
    CHECK(status.statusText() == QStringLiteral("TARGET ACQUIRED"));
}

TEST_CASE("The IMU strip shows tilt, turning, the board's up axis and the gate") {
    ViewModels::ImuStatusViewModel imu;
    CHECK(!imu.isAvailable());
    CHECK(imu.gate() == QStringLiteral("NO DATA"));

    // Nose 10° up, the right side a hair low: roll -0.004° rounds to -0.0, shown as +0.0.
    const float pitch = 10.0f * std::numbers::pi_v<float> / 180.0f;
    const MMWave::Vector3 up{0.00007f, std::sin(pitch), std::cos(pitch)};
    Imu::Gating::ImuFrameState state;
    state.state = Imu::Gating::MotionGateState::Settling;
    state.tilt = Imu::Fusion::TiltCorrection::fromUp(up);
    state.window.sampleCount = 30;
    state.window.meanAngularRate = 12.34f;
    state.window.meanUp = up;
    imu.show(state);
    CHECK(imu.isAvailable());
    CHECK(imu.pitch() == QStringLiteral("+10.0°"));
    CHECK(imu.roll() == QStringLiteral("+0.0°"));
    CHECK(imu.turning() == QStringLiteral("12.3 °/s"));
    // The handheld mounting has the board's Y arrow up.
    CHECK(imu.upAxis() == QStringLiteral("+Y"));
    CHECK(imu.gate() == QStringLiteral("SETTLING"));
    CHECK(!imu.isSteady());
    CHECK(!imu.isMoving());

    state.state = Imu::Gating::MotionGateState::Moving;
    state.tilt = Imu::Fusion::TiltCorrection::fromUp({-0.5f, 0, 0.866f});
    state.window.meanUp = {0, 0, -1};
    imu.show(state);
    CHECK(imu.roll() == QStringLiteral("+30.0°"));
    CHECK(imu.upAxis() == QStringLiteral("-Y"));
    CHECK(imu.isMoving());

    state.state = Imu::Gating::MotionGateState::Unavailable;
    imu.show(state);
    CHECK(imu.isAvailable());
    CHECK(imu.pitch() == QStringLiteral("-"));
    CHECK(imu.gate() == QStringLiteral("NO DATA"));

    imu.show(std::nullopt);
    CHECK(!imu.isAvailable());
}

TEST_CASE("RadarData uses accepted frames, leaves refused ones out, and still reports damage") {
    app();
    namespace Gating = Sarwave::Core::Imu::Gating;
    using MMWave::TimeSpans::fromSeconds;
    Models::RadarData data(fromSeconds(10));
    ViewModels::MinimalGuiViewModel viewModel(data);
    int updates = 0;
    const int token = data.subscribe([&] { updates++; });

    // No IMU: applied as it is.
    data.update(FrameBuilder().cloud(2).frame(0), fromSeconds(0), std::nullopt);
    CHECK_EQ(data.pointClouds().size(), std::size_t{1});
    CHECK(!data.holdSteady());
    CHECK(!data.imu().has_value());

    // Refused (moving): left out, but time still moves on. HOLD STEADY only once the gate has
    // refused frames for longer than a knock would keep it refusing.
    Gating::ImuFrameState moving{.state = Gating::MotionGateState::Moving, .accepted = false};
    data.update(FrameBuilder().cloud(2).frame(1), fromSeconds(1), moving);
    CHECK_EQ(data.pointClouds().size(), std::size_t{1});
    CHECK(!data.holdSteady());
    CHECK(data.lastUpdate() == fromSeconds(1));
    CHECK(data.imu().has_value());
    data.update(FrameBuilder().cloud(2).frame(2), fromSeconds(1.7), moving);
    CHECK(data.holdSteady());
    processEvents();
    CHECK(viewModel.status()->holdSteady());
    CHECK(viewModel.status()->statusText() == QStringLiteral("HOLD STEADY"));

    // A damaged frame is reported even while refused.
    CHECK_THROWS_AS(
        data.update(FrameBuilder().cloud(2).detectedObjects(5).frame(3), fromSeconds(1.8), moving),
        MMWave::Exceptions::FrameEntryMalformedException);

    // Accepted: used again, and the warning clears.
    Gating::ImuFrameState steady{.state = Gating::MotionGateState::Steady, .accepted = true};
    data.update(FrameBuilder().cloud(1).frame(4), fromSeconds(2), steady);
    CHECK(!data.holdSteady());
    CHECK_EQ(data.pointClouds().size(), std::size_t{2});
    CHECK_EQ(updates, 4);
    data.unsubscribe(token);
}

TEST_CASE("The session file lives in the user's local application data") {
    const auto& path = Sarwave::DevEnv::Gui::RunRecorded::sessionFile();
    CHECK(path.is_absolute());
    CHECK_EQ(path.filename().string(), "default_session.swrec");
    CHECK_EQ(path.parent_path().filename().string(), "Sarwave");
    CHECK(std::filesystem::is_directory(path.parent_path()));
    CHECK(Sarwave::Integration::PeopleTracking::parseImuMode("Required") ==
          Sarwave::Integration::PeopleTracking::ImuMode::Required);
    CHECK(!Sarwave::Integration::PeopleTracking::parseImuMode("maybe"));
}

TEST_CASE("RadarDataOverlay times its paints for the live timing report") {
    app();
    Views::Radar::RadarDataOverlay overlay;
    Views::Radar::RadarDataOverlay::paintTimes().take();

    QImage image(100, 100, QImage::Format_ARGB32_Premultiplied);
    for (int i = 0; i < 3; i++) {
        QPainter painter(&image);
        overlay.paint(&painter);
    }

    const auto figures = Views::Radar::RadarDataOverlay::paintTimes().take();
    CHECK_EQ(figures.count, 3);
    CHECK(figures.longest >= figures.average());
    CHECK_EQ(Views::Radar::RadarDataOverlay::paintTimes().take().count, 0);
}
