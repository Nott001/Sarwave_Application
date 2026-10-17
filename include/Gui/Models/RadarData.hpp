#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "MMWave/Common/Numerics.hpp"
#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Configured/PeopleTracking/Association/CombinedManager.hpp"
#include "MMWave/Streaming/Frame.hpp"

namespace Sarwave::Core::Gui::Models {

/// The radar state a dashboard shows: a CombinedManager fed one frame at a time, plus the
/// `Updated` notification the view models listen to. Not a QObject and not tied to any thread:
/// whoever calls update() (the capture or playback thread) is the only one that may read it.
class RadarData {
   public:
    explicit RadarData(MMWave::TimeSpan duration);

    RadarData(const RadarData&) = delete;
    RadarData& operator=(const RadarData&) = delete;

    using Handler = std::function<void()>;

    /// The C# `Updated` event. Raised once per update(), on the calling thread, after the
    /// frame has been applied. Handlers may read this object synchronously; marshal to the UI
    /// thread before touching controls. Returns a token for unsubscribe().
    int subscribe(Handler handler);

    /// After this returns, the handler is not running and never will again. Waits for a
    /// running raise on another thread to finish, so a handler must never block on the thread
    /// that unsubscribes. A handler may unsubscribe itself (or another one).
    void unsubscribe(int token);

    /// The timestamp passed to the latest update().
    [[nodiscard]] MMWave::TimeSpan lastUpdate() const {
        return m_lastUpdate;
    }

    [[nodiscard]] bool presence() const {
        return m_manager.presence();
    }

    [[nodiscard]] std::vector<const MMWave::Configured::PeopleTracking::Association::TargetObject*>
    allTargetObjects() const {
        return m_manager.allTargetObjects();
    }

    [[nodiscard]] const MMWave::Configured::PeopleTracking::Association::PointEntryRange::Entries&
    pointClouds() const {
        return m_manager.pointClouds();
    }

    MMWave::Configured::PeopleTracking::Association::CombinedManager& innerManager() {
        return m_manager;
    }

    [[nodiscard]] MMWave::TimeSpan duration() const {
        return m_manager.duration;
    }

    void setDuration(MMWave::TimeSpan value) {
        m_manager.duration = value;
    }

    /// The operator should hold the device steady: motion gating is refusing frames. Set by
    /// whoever gates the frames, before the update() or skipFrame() it applies to; reported with
    /// that frame's Updated.
    [[nodiscard]] bool holdSteady() const {
        return m_holdSteady;
    }

    void setHoldSteady(bool value) {
        m_holdSteady = value;
    }

    /// Applies the frame, then raises Updated synchronously on the calling thread (any thread).
    void update(const MMWave::Streaming::Frame& frame, MMWave::TimeSpan timestamp);

    /// leveling: rotation applied to the frame's points, e.g. an IMU tilt correction.
    void update(const MMWave::Streaming::Frame& frame, MMWave::TimeSpan timestamp,
                MMWave::Quaternion leveling);

    /// For a frame that arrived but could not be used; see CombinedManager::skipFrame. Raises
    /// Updated like any other frame.
    void skipFrame(MMWave::TimeSpan timestamp);

   private:
    struct Subscriber {
        int token;
        Handler handler;
        std::atomic<bool> removed = false;
    };

    void raiseUpdated();
    void onUpdated(MMWave::TimeSpan timestamp);

    MMWave::Configured::PeopleTracking::Association::CombinedManager m_manager;
    MMWave::TimeSpan m_lastUpdate{};
    bool m_holdSteady = false;

    std::mutex m_subscribersMutex;  // guards m_subscribers and m_nextToken
    std::vector<std::shared_ptr<Subscriber>> m_subscribers;
    int m_nextToken = 1;

    // Held while handlers run; unsubscribe() takes it to wait out a raise on another thread.
    // Recursive, so a handler can unsubscribe from inside the raise.
    std::recursive_mutex m_raiseMutex;
};

}  // namespace Sarwave::Core::Gui::Models
