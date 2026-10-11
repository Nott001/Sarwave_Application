#pragma once

#include <QtQml/qqmlregistration.h>

#include <QObject>
#include <functional>

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

   protected:
    /// Runs `action` on the UI thread: inline if already there, posted otherwise. The post is
    /// tied to this object, so it is dropped if the view model is destroyed before it runs.
    void onUiThread(std::function<void()> action);
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
