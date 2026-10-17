#pragma once

#include <QtQml/qqmlregistration.h>

#include <QMetaObject>
#include <QObject>
#include <QThread>
#include <functional>
#include <mutex>
#include <optional>
#include <utility>

#include "Gui/Models/RadarData.hpp"

namespace Sarwave::Core::Gui::ViewModels {

/// Stands in for Avalonia's Design.IsDesignMode: a view model constructed with it fills itself
/// with the previewer's sample data.
struct DesignModeTag {
    explicit DesignModeTag() = default;
};
inline constexpr DesignModeTag DesignMode{};

/// Minimal property-change base (the C# INotifyPropertyChanged one). Each property has its own
/// NOTIFY signal in the derived classes. View models live on the GUI thread.
class ViewModelBase : public QObject {
    Q_OBJECT
    QML_ANONYMOUS

   public:
    using QObject::QObject;
};

/// Hands the newest value to the UI thread, replacing one still waiting there instead of queueing
/// behind it. Each value is a whole state (a snapshot, a target list), not an event, so only the
/// latest matters: a GUI that draws slower than frames arrive then skips the stale ones instead
/// of falling further and further behind and holding all of them in memory.
///
/// `apply` runs on `context`'s thread: inline if post() is called there, queued otherwise. The
/// queued call is tied to `context`, so it is dropped if that is destroyed first. Declare this
/// before the RadarDataSubscription that posts to it.
template <class T>
class LatestOnUiThread {
   public:
    LatestOnUiThread(QObject* context, std::function<void(T)> apply)
        : m_context(context), m_apply(std::move(apply)) {
    }

    LatestOnUiThread(const LatestOnUiThread&) = delete;
    LatestOnUiThread& operator=(const LatestOnUiThread&) = delete;

    void post(T value) {
        if (QThread::currentThread() == m_context->thread()) {
            // Anything still waiting is older than this.
            {
                std::lock_guard lock(m_gate);
                m_pending.reset();
            }
            m_apply(std::move(value));
            return;
        }

        {
            std::lock_guard lock(m_gate);
            const bool alreadyPosted = m_pending.has_value();
            m_pending = std::move(value);
            if (alreadyPosted) return;  // the call already queued will take this one
        }
        QMetaObject::invokeMethod(m_context, [this] { deliver(); }, Qt::QueuedConnection);
    }

   private:
    void deliver() {
        std::optional<T> value;
        {
            std::lock_guard lock(m_gate);
            value.swap(m_pending);
        }
        if (value) m_apply(std::move(*value));
    }

    QObject* m_context;
    std::function<void(T)> m_apply;
    std::mutex m_gate;
    std::optional<T> m_pending;  // guarded by m_gate; set while a delivery is queued
};

/// The status header's two values, copied off the capture thread.
struct StatusValues {
    bool presence = false;
    bool holdSteady = false;
};

/// One subscription to RadarData's Updated event, ended (waiting out a running handler) when this
/// is destroyed. Declare it as the last member of the view model that owns it, so it is ended
/// before anything else of the view model goes away.
class RadarDataSubscription {
   public:
    RadarDataSubscription() = default;
    RadarDataSubscription(Models::RadarData& data, Models::RadarData::Handler handler);
    ~RadarDataSubscription();

    RadarDataSubscription(const RadarDataSubscription&) = delete;
    RadarDataSubscription& operator=(const RadarDataSubscription&) = delete;
    RadarDataSubscription(RadarDataSubscription&& other) noexcept;
    RadarDataSubscription& operator=(RadarDataSubscription&& other) noexcept;

    void reset();

   private:
    Models::RadarData* m_data = nullptr;
    int m_token = 0;
};

}  // namespace Sarwave::Core::Gui::ViewModels
