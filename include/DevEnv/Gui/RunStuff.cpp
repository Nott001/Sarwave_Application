#include <QDebug>
#include <QEventLoop>
#include <QMetaObject>
#include <QPointer>
#include <QQuickWindow>
#include <QTimer>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <thread>

#include "DevEnv/ConfigMemory.hpp"
#include "DevEnv/Gui/CfgDialogue.hpp"
#include "DevEnv/Gui/PortDialogue.hpp"
#include "DevEnv/Gui/Run.hpp"
#include "Gui/Models/RadarData.hpp"
#include "Gui/Views/GuiHost.hpp"
#include "Gui/Views/Radar/RadarDataOverlay.hpp"
#include "Integration/PeopleTracking/ImuFrames.hpp"
#include "MMWave/Common/DurationStats.hpp"
#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Configured/PeopleTracking/DebugLogger.hpp"
#include "MMWave/Exceptions/FrameMalformedException.hpp"
#include "MMWave/Porting/Context.hpp"
#include "MMWave/Recording/FrameRecorder.hpp"
#include "MMWave/Streaming/ConstantCapture.hpp"

namespace Sarwave::DevEnv::Gui {
namespace {

using namespace Sarwave::Core;
namespace CoreGui = Sarwave::Core::Gui;
using Integration::PeopleTracking::ImuFrames;
using MMWave::Configured::PeopleTracking::DebugLogger;
using MMWave::Exceptions::FrameEntryMalformedException;
using MMWave::Exceptions::FrameMalformedException;
using MMWave::Exceptions::FrameReaderMalformedException;
using MMWave::Porting::Context;
using MMWave::Recording::FrameRecorder;
using MMWave::Streaming::ConstantCapture;

/// How long point clouds stay on the radar grid.
const MMWave::TimeSpan PointRetention = MMWave::TimeSpans::fromSeconds(1);

/// Real-time priorities for the threads that must never wait: the data-port reader first, then
/// the IMU sampler. Both sleep almost all the time. Kept below the kernel's own interrupt
/// threads (50).
constexpr int ReaderPriority = 30;
constexpr int ImuPriority = 20;

/// How often RunStuff::timingReport prints.
constexpr std::chrono::seconds TimingInterval{10};

// Process-lifetime state, as the C# static fields; destroyed in reverse order at exit, so the
// capture (last) stops before the IMU, recorder, data and ports it uses.
std::unique_ptr<CoreGui::Models::RadarData> g_data;
std::unique_ptr<Context> g_ctx;
std::unique_ptr<FrameRecorder> g_recorder;
std::unique_ptr<Imu::ImuSession> g_imu;
DebugLogger g_debugLogger(DebugLogger::Config{.hideBytes = true});
std::unique_ptr<ConstantCapture> g_capture;

// RunStuff::timingReport's figures, added on the worker thread, taken on the GUI thread.
MMWave::DurationStats g_processTimes;  // the frame callback, start to end
MMWave::DurationStats g_waitTimes;     // from a frame's arrival to its callback starting

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

/// TimeSpan's hh\:mm\:ss\.fff.
std::string formatElapsed(MMWave::TimeSpan elapsed) {
    const std::int64_t ms = elapsed.count() / MMWave::TimeSpans::TicksPerMillisecond;
    char text[32];
    std::snprintf(text, sizeof text, "%02lld:%02lld:%02lld.%03lld",
                  static_cast<long long>(ms / 3'600'000 % 24),
                  static_cast<long long>(ms / 60'000 % 60), static_cast<long long>(ms / 1000 % 60),
                  static_cast<long long>(ms % 1000));
    return text;
}

/// Writes the error to the terminal (standard error). A malformed frame gets one line: its type,
/// cause and message say everything. Anything else is written with its type and message.
void printError(const std::string& what, const std::exception& exception,
                MMWave::TimeSpan elapsed) {
    std::string text;
    if (const auto* reader = dynamic_cast<const FrameReaderMalformedException*>(&exception)) {
        text = ": FrameReaderMalformedException (" + MMWave::Exceptions::toString(reader->cause()) +
               "): " + reader->what();
    } else if (const auto* entry = dynamic_cast<const FrameEntryMalformedException*>(&exception)) {
        text = ": FrameEntryMalformedException (" + MMWave::Exceptions::toString(entry->cause()) +
               "): " + entry->what();
    } else if (dynamic_cast<const FrameMalformedException*>(&exception)) {
        text = std::string(": FrameMalformedException: ") + exception.what();
    } else {
        text = ": " + dotNetTypeName(exception) + ": " + exception.what();
    }

    std::cerr << "[" << formatElapsed(elapsed) << "] " << what << text << std::endl;
}

void printError(const std::string& what, const std::exception_ptr& error,
                MMWave::TimeSpan elapsed) {
    try {
        std::rethrow_exception(error);
    } catch (const std::exception& ex) {
        printError(what, ex, elapsed);
    } catch (...) {
        std::cerr << "[" << formatElapsed(elapsed) << "] " << what << ": unknown exception"
                  << std::endl;
    }
}

/// "19 us" below a millisecond, "0.62 ms" above.
std::string formatDuration(MMWave::DurationStats::Duration duration) {
    char text[32];
    const double us = std::chrono::duration<double, std::micro>(duration).count();
    if (us < 1000) {
        std::snprintf(text, sizeof text, "%.0f us", us);
    } else {
        std::snprintf(text, sizeof text, "%.2f ms", us / 1000);
    }
    return text;
}

/// One line of RunStuff::timingReport, covering the time since the last.
void printTimings(MMWave::TimeSpan elapsed) {
    const auto process = g_processTimes.take();
    const auto wait = g_waitTimes.take();
    const auto paint = CoreGui::Views::Radar::RadarDataOverlay::paintTimes().take();
    std::cerr << "[" << formatElapsed(elapsed) << "] Timing, last " << TimingInterval.count()
              << " s: " << process.count << " frames, processing avg "
              << formatDuration(process.average()) << " / max " << formatDuration(process.longest)
              << ", waiting avg " << formatDuration(wait.average()) << " / max "
              << formatDuration(wait.longest) << "; drawn " << paint.count << " times, avg "
              << formatDuration(paint.average()) << " / max " << formatDuration(paint.longest)
              << std::endl;
}

std::string trim(const std::string& text) {
    const auto first = text.find_first_not_of(" \t\n\v\f\r");
    if (first == std::string::npos) return {};
    const auto last = text.find_last_not_of(" \t\n\v\f\r");
    return text.substr(first, last - first + 1);
}

}  // namespace

bool RunStuff::timingReport = false;

void RunStuff::onOpened(QQuickWindow& window, QQmlEngine& engine, bool printFrames) {
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
        g_data->setTrackerFrame(ImuFrames::loadTrackerFrame(*cfgPath));

        // Frame recording
        g_recorder = std::make_unique<FrameRecorder>(RunRecorded::sessionFile());

        // Opened before the capture so the frame callback below can ask it about every frame.
        g_imu = ImuFrames::tryOpen();
        Imu::ImuSession* imu = g_imu.get();
        const std::filesystem::path imuRecordingPath =
            Imu::Recording::ImuRecording::sidecarPath(RunRecorded::sessionFile());
        if (!imu) {
            // A previous session's IMU data must not be replayed against this session's frames.
            std::error_code ignored;
            std::filesystem::remove(imuRecordingPath, ignored);
        }

        // Frames arrive on the capture's worker thread. RadarData is only ever touched from
        // there; the view models copy what they need and hand it to the GUI thread themselves.
        QPointer<QQuickWindow> windowRef(&window);
        g_capture = std::make_unique<ConstantCapture>(*g_ctx);
        ConstantCapture* capture = g_capture.get();

        // Covers every way the capture can end by failing, not only an exception from the
        // callbacks below: a read error on the data port ends it without touching either.
        capture->onCompleted = [capture, windowRef](std::exception_ptr error) {
            if (!error) return;

            printError("Capture stopped", error, capture->elapsed());
            QString message;
            try {
                std::rethrow_exception(error);
            } catch (const std::exception& ex) {
                message = QString::fromStdString(ex.what());
            } catch (...) {
            }
            const QString title = QStringLiteral("Capture stopped: ") + message;
            QMetaObject::invokeMethod(
                windowRef.data(),
                [windowRef, title] {
                    if (windowRef) windowRef->setTitle(title);
                },
                Qt::QueuedConnection);
        };

        // The reader thread queues frames while the worker runs the callbacks; if the worker
        // ever falls so far behind that the queue overflows, say so -- that is "processing too
        // slow", not a corrupted link.
        capture->onDropped = [](int count, MMWave::TimeSpan elapsed) {
            std::cerr << "[" << formatElapsed(elapsed) << "] Processing too slow: " << count
                      << " frame(s) dropped unprocessed" << std::endl;
        };

        capture->readerPriority = ReaderPriority;
        const bool timing = timingReport;
        capture->beginConstantCapture(
            [printFrames, imu, timing, capture](const MMWave::Streaming::Frame& frame,
                                                MMWave::TimeSpan elapsed) {
                // `elapsed` is when the frame arrived, so the time since is how long it queued.
                std::chrono::steady_clock::time_point start;
                if (timing) {
                    start = std::chrono::steady_clock::now();
                    g_waitTimes.add(capture->elapsed() - elapsed);
                }

                // Nothing is caught here. A malformed frame goes to the callback below and the
                // capture carries on; anything else ends the capture and is reported when it
                // completes.
                if (printFrames) g_debugLogger.printFrameDataVerbose(frame, elapsed);
                g_recorder->record(frame, elapsed);

                // Frames the gate refuses (device moving or settling) are left out, but what is
                // kept from earlier ones is still moved with the device.
                std::optional<Imu::Gating::ImuFrameState> state;
                if (imu) state = imu->evaluate(elapsed);
                g_data->update(frame, elapsed, state);

                if (timing) g_processTimes.add(std::chrono::steady_clock::now() - start);
            },
            [](const FrameMalformedException& exception, MMWave::TimeSpan elapsed) {
                printError("Malformed frame", exception, elapsed);
                g_recorder->recordMalformed(exception, elapsed);

                // The frame after this one carries indexes for a cloud that was never added.
                g_data->skipFrame(elapsed);
            });

        if (!capture->readerIsRealtime()) {
            std::cerr << "[" << formatElapsed(capture->elapsed())
                      << "] Note: the data-port reader runs at normal priority. To let it run "
                         "real-time, allow it once with\n"
                         "    echo \"$USER - rtprio "
                      << ReaderPriority
                      << "\" | sudo tee /etc/security/limits.d/sarwave.conf\n"
                         "and log in again."
                      << std::endl;
        }

        if (timing) {
            // On the GUI thread, so printing never holds up the reader or the worker.
            CoreGui::Views::Radar::RadarDataOverlay::paintTimes().take();  // drop startup paints
            auto* timer = new QTimer(&window);
            QObject::connect(timer, &QTimer::timeout, timer,
                             [capture] { printTimings(capture->elapsed()); });
            timer->start(TimingInterval);
        }

        if (imu) {
            // Stamped on the capture's own clock, which beginConstantCapture has just started,
            // so IMU samples and frames line up. If the IMU fails, from here on or later, the
            // radar carries on and its frames count as unknown motion.
            try {
                imu->sampler().priority = ImuPriority;
                imu->sampler().onCompleted = [capture](std::exception_ptr error) {
                    if (error) printError("IMU stopped", error, capture->elapsed());
                };
                imu->start([capture] { return capture->elapsed(); }, imuRecordingPath);
            } catch (const IOException& ex) {
                printError("IMU failed to start", ex, capture->elapsed());
            }
        }

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
        printError("Startup failed", ex,
                   g_capture ? g_capture->elapsed() : MMWave::TimeSpan::zero());
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

        // After the capture: its callback asks the IMU about every frame.
        if (g_imu) g_imu->dispose();

        try {
            g_ctx->sendCliCommand(Context::SensorStopCommand);
        } catch (const std::exception&) {
            // Port already gone (unplugged); nothing left to stop.
        }
    }

    g_ctx->dispose();
}

}  // namespace Sarwave::DevEnv::Gui
