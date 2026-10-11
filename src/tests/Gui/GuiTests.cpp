#include <QCoreApplication>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QTimer>
#include <cmath>
#include <memory>

#include "../Core/FrameBuilder.hpp"
#include "DevEnv/ConfigMemory.hpp"
#include "DevEnv/DevEnv.hpp"
#include "DevEnv/Gui/CfgDialogue.hpp"
#include "DevEnv/Gui/PortDialogue.hpp"
#include "Gui/Common/DotNetRandom.hpp"
#include "Gui/Models/RadarData.hpp"
#include "Gui/ViewModels/Object/ObjectsPanelViewModel.hpp"
#include "Gui/ViewModels/Radar/RadarGridViewModel.hpp"
#include "Gui/Views/GuiHost.hpp"
#include "Gui/Views/Radar/ColourManager.hpp"
#include "Gui/Views/Radar/RadarGeometry.hpp"
#include "Gui/Views/StringFormat.hpp"
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
    data.update(FrameBuilder().targets({1}).cloud(3).frame(), MMWave::TimeSpan(0));
    data.update(FrameBuilder().targets({1}).indexes({1, 253, 1}).cloud(2).frame(),
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
        data.update(FrameBuilder().targets({5, 6}).presence(true).frame(), MMWave::TimeSpan(0));
    });
    worker.join();
    processEvents();
    CHECK_EQ(panel->count(), 2);
    panel.reset();  // unsubscribes; later updates must not touch it
    data.update(FrameBuilder().targets({5}).frame(), MMWave::TimeSpan(100));
    processEvents();
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
    data.update(FrameBuilder().targets({1}).cloud(4).presence(true).frame(), MMWave::TimeSpan(0));
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
