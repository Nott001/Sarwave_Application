#include <QDebug>
#include <QEventLoop>
#include <QMetaObject>
#include <QPointer>
#include <QQuickWindow>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>

#include "DevEnv/ConfigMemory.hpp"
#include "DevEnv/Gui/CfgDialogue.hpp"
#include "DevEnv/Gui/PortDialogue.hpp"
#include "DevEnv/Gui/Run.hpp"
#include "Gui/Models/RadarData.hpp"
#include "Gui/Views/GuiHost.hpp"
#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Configured/PeopleTracking/DebugLogger.hpp"
#include "MMWave/Porting/Context.hpp"
#include "MMWave/Recording/FrameRecorder.hpp"
#include "MMWave/Streaming/ConstantCapture.hpp"

namespace Sarwave::DevEnv::Gui {
namespace {

using namespace Sarwave::Core;
namespace CoreGui = Sarwave::Core::Gui;
using MMWave::Configured::PeopleTracking::DebugLogger;
using MMWave::Porting::Context;
using MMWave::Recording::FrameRecorder;
using MMWave::Streaming::ConstantCapture;

/// How long point clouds stay on the radar grid.
const MMWave::TimeSpan PointRetention = MMWave::TimeSpans::fromSeconds(1);

// Process-lifetime state, as the C# static fields; destroyed in reverse order at exit, so the
// capture (last) stops before the recorder, data and ports it uses.
std::unique_ptr<CoreGui::Models::RadarData> g_data;
std::unique_ptr<Context> g_ctx;
std::unique_ptr<FrameRecorder> g_recorder;
DebugLogger g_debugLogger(DebugLogger::Config{.hideBytes = true});
std::unique_ptr<ConstantCapture> g_capture;

/// `await Task.Run(work)`: runs blocking work on another thread while the GUI keeps processing
/// events, then returns its result (or rethrows its exception).
template <class Work>
auto runInBackground(Work work) -> decltype(work()) {
    using R = decltype(work());
    std::optional<R> result;
    std::exception_ptr error;
    QEventLoop loop;
    QObject receiver;
    std::thread worker([&] {
        try {
            result.emplace(work());
        } catch (...) {
            error = std::current_exception();
        }
        QMetaObject::invokeMethod(&receiver, [&loop] { loop.quit(); }, Qt::QueuedConnection);
    });
    loop.exec();
    worker.join();
    if (error) std::rethrow_exception(error);
    return std::move(*result);
}

/// Environment.FailFast(message): report and terminate immediately.
[[noreturn]] void failFast(const std::string& message) {
    std::cerr << "Process terminated. " << message << std::endl;
    std::abort();
}

std::string trim(const std::string& text) {
    const auto first = text.find_first_not_of(" \t\n\v\f\r");
    if (first == std::string::npos) return {};
    const auto last = text.find_last_not_of(" \t\n\v\f\r");
    return text.substr(first, last - first + 1);
}

}  // namespace

void RunStuff::onOpened(QQuickWindow& window, QQmlEngine& engine, bool save) {
    try {
        // The dashboard gets its own view model up front.
        g_data = std::make_unique<CoreGui::Models::RadarData>(PointRetention);
        CoreGui::Views::showMinimalGui(window, engine, *g_data);

        std::unique_ptr<Context> ctx = PortDialogue::showAsync(&window, engine);
        if (!ctx) return;
        g_ctx = std::move(ctx);  // disposed in onClosed from here on

        std::optional<std::string> cfgPath = ConfigMemory::getConfig();
        cfgPath = CfgDialogue::showAsync(&window, engine, g_ctx.get(), cfgPath);
        if (!cfgPath) return;
        ConfigMemory::saveConfigLocation(*cfgPath);

        // Frame recording
        g_recorder = std::make_unique<FrameRecorder>(RunRecorded::sessionFile());

        // Frames arrive on the capture thread. RadarData is only ever touched from there; the
        // view models copy what they need and hand it to the GUI thread themselves.
        const auto clockStart = std::chrono::steady_clock::now();
        QPointer<QQuickWindow> windowRef(&window);
        g_capture = std::make_unique<ConstantCapture>(*g_ctx);
        g_capture->beginConstantCapture(
            [clockStart, windowRef, save](const MMWave::Streaming::Frame& frame) {
                try {
                    const MMWave::TimeSpan elapsed =
                        MMWave::TimeSpans::from(std::chrono::steady_clock::now() - clockStart);
                    if (save) g_debugLogger.printFrameDataVerbose(frame, elapsed);
                    g_recorder->record(frame, elapsed);
                    g_data->update(frame, elapsed);
                } catch (const std::exception& ex) {
                    // Rethrown so ConstantCapture stops and records it; shown so it isn't silent.
                    const QString title =
                        QStringLiteral("Capture stopped: ") + QString::fromStdString(ex.what());
                    QMetaObject::invokeMethod(
                        windowRef.data(),
                        [windowRef, title] {
                            if (windowRef) windowRef->setTitle(title);
                        },
                        Qt::QueuedConnection);
                    qDebug().noquote() << ex.what();
                    throw;
                }
            });

        // CfgDialogue sends the cfg without its sensorStart line, so the capture above is
        // already listening when the first frame goes out.
        Context* ctxRaw = g_ctx.get();
        const std::string response = runInBackground([ctxRaw] {
            return ctxRaw->sendCliCommand(Context::SensorStartCommand, std::chrono::seconds(5));
        });
        if (response.find("Done") == std::string::npos) {
            throw InvalidOperationException("sensorStart failed or timed out: " + trim(response));
        }
    } catch (const std::exception& ex) {
        failFast(ex.what());
    }
}

void RunStuff::onClosed() {
    if (!g_ctx) return;

    if (g_capture) {
        try {
            g_capture->stopConstantCapture(std::chrono::seconds(1));
            if (g_recorder) g_recorder->dispose();
        } catch (const OperationCanceledException&) {
            // The pending read ignored cancellation; closing the port below ends it.
        }

        try {
            g_ctx->sendCliCommand(Context::SensorStopCommand);
        } catch (const std::exception&) {
            // Port already gone (unplugged); nothing left to stop.
        }
    }

    g_ctx->dispose();
}

}  // namespace Sarwave::DevEnv::Gui
