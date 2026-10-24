#include <QMetaObject>
#include <QPointer>
#include <QQuickWindow>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

#include "DevEnv/Gui/LanDialogue.hpp"
#include "DevEnv/Gui/Run.hpp"
#include "Gui/Models/RadarData.hpp"
#include "Gui/Views/GuiHost.hpp"
#include "Imu/Fusion/ImuProcessor.hpp"
#include "Imu/Gating/MotionGate.hpp"
#include "Imu/Timeline/ImuTimeline.hpp"
#include "Integration/PeopleTracking/ImuFrames.hpp"
#include "Integration/PeopleTracking/Lan/LanReceiver.hpp"
#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Exceptions/FrameMalformedException.hpp"

namespace Sarwave::DevEnv::Gui {
namespace {

using namespace Sarwave::Core;
namespace CoreGui = Sarwave::Core::Gui;
using Integration::PeopleTracking::ImuFrames;
using Integration::PeopleTracking::Lan::LanReceiver;

/// How long point clouds stay on the radar grid.
const MMWave::TimeSpan PointRetention = MMWave::TimeSpans::fromSeconds(1);

/// The IMU pipeline the received samples go through, as a replay's: this app's own
/// ImuFrames::options().
struct ImuPipeline {
    Imu::Fusion::ImuProcessor processor{ImuFrames::options().mounting,
                                        ImuFrames::options().processing};
    Imu::Timeline::ImuTimeline timeline{ImuFrames::options().timelineCapacity};
    Imu::Gating::MotionGate gate{timeline, ImuFrames::options().gate};
    std::vector<Imu::ImuState> states;
};

// Process-lifetime state, as the C# static fields. Destroyed at exit in reverse order, so the
// receiver (last) stops before the data and the pipeline its callbacks use.
std::unique_ptr<CoreGui::Models::RadarData> g_data;
std::unique_ptr<ImuPipeline> g_imu;
std::unique_ptr<LanReceiver> g_receiver;

/// TimeSpan's hh\:mm\:ss\.fff.
std::string formatTimestamp(MMWave::TimeSpan timestamp) {
    const std::int64_t ms = timestamp.count() / MMWave::TimeSpans::TicksPerMillisecond;
    char text[32];
    std::snprintf(text, sizeof text, "%02lld:%02lld:%02lld.%03lld",
                  static_cast<long long>(ms / 3'600'000 % 24),
                  static_cast<long long>(ms / 60'000 % 60), static_cast<long long>(ms / 1000 % 60),
                  static_cast<long long>(ms % 1000));
    return text;
}

/// GetType().Name: the type's name without its namespace.
std::string typeName(const std::exception& exception) {
    const std::string full = dotNetTypeName(exception);
    return full.substr(full.rfind('.') + 1);
}

}  // namespace

void RunLan::onOpened(QQuickWindow& window, QQmlEngine& engine) {
    try {
        // The dashboard gets its own view model up front.
        g_data = std::make_unique<CoreGui::Models::RadarData>(PointRetention);
        CoreGui::Views::showMaximalGui(window, engine, *g_data);

        const auto sender = LanDialogue::showAsync(&window, engine, LanDialogue::Role::Receive);
        if (!sender) return;

        g_imu = std::make_unique<ImuPipeline>();
        g_receiver = std::make_unique<LanReceiver>(sender->host, sender->port);

        // Everything below runs on the receiver's thread, one message at a time: RadarData and
        // the IMU pipeline are only ever touched from there.
        QPointer<QQuickWindow> windowRef(&window);
        g_receiver->statusChanged = [windowRef](bool connected, const std::string& message) {
            std::cerr << message << std::endl;
            const QString title = (connected ? QStringLiteral("Sarwave Wireless: ")
                                             : QStringLiteral("Sarwave Wireless (waiting): ")) +
                                  QString::fromStdString(message);
            QMetaObject::invokeMethod(
                windowRef.data(),
                [windowRef, title] {
                    if (windowRef) windowRef->setTitle(title);
                },
                Qt::QueuedConnection);
        };

        CoreGui::Models::RadarData* data = g_data.get();
        ImuPipeline* imu = g_imu.get();
        g_receiver->start(
            [data, imu](const std::string& cfgText) {
                // A new connection is a new session, on a clock of its own.
                data->setTrackerFrame(ImuFrames::parseTrackerFrame(cfgText));
                imu->processor.reset();
                imu->timeline.clear();
                imu->gate.reset();
            },
            [data, imu](const MMWave::Streaming::Frame& frame, MMWave::TimeSpan timestamp) {
                // Without IMU data from the sender the gate finds nothing, and the frame is
                // applied as it is, as when the live app has no IMU.
                data->update(frame, timestamp, imu->gate.evaluate(timestamp));
            },
            [data](const MMWave::Exceptions::FrameMalformedException& exception,
                   MMWave::TimeSpan timestamp) {
                std::cerr << "[" << formatTimestamp(timestamp)
                          << "] Malformed frame: " << typeName(exception) << ": "
                          << exception.what() << std::endl;

                // The frame after this one carries indexes for a cloud that was never added.
                data->skipFrame(timestamp);
            },
            [imu](std::span<const Imu::ImuSample> samples) {
                imu->states.resize(samples.size());
                for (std::size_t i = 0; i < samples.size(); ++i)
                    imu->states[i] = imu->processor.process(samples[i]);
                imu->timeline.append(imu->states);
            });
    } catch (const std::exception& ex) {
        std::cerr << "Startup failed: " << dotNetTypeName(ex) << ": " << ex.what() << std::endl;
        std::cerr << "Process terminated. " << ex.what() << std::endl;
        std::abort();
    }
}

void RunLan::onClosed() {
    g_receiver.reset();  // stops it, waiting out a callback in progress
}

}  // namespace Sarwave::DevEnv::Gui
