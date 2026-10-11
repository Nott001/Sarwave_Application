#include "Gui/ViewModels/ViewModelBase.hpp"

#include <QMetaObject>
#include <QThread>
#include <utility>

namespace Sarwave::Core::Gui::ViewModels {

void ViewModelBase::onUiThread(std::function<void()> action) {
    if (QThread::currentThread() == thread()) {
        action();
    } else {
        QMetaObject::invokeMethod(this, std::move(action), Qt::QueuedConnection);
    }
}

RadarDataSubscription::RadarDataSubscription(Models::RadarData& data,
                                             Models::RadarData::Handler handler)
    : m_data(&data), m_token(data.subscribe(std::move(handler))) {
}

RadarDataSubscription::~RadarDataSubscription() {
    reset();
}

RadarDataSubscription::RadarDataSubscription(RadarDataSubscription&& other) noexcept
    : m_data(std::exchange(other.m_data, nullptr)), m_token(std::exchange(other.m_token, 0)) {
}

RadarDataSubscription& RadarDataSubscription::operator=(RadarDataSubscription&& other) noexcept {
    if (this != &other) {
        reset();
        m_data = std::exchange(other.m_data, nullptr);
        m_token = std::exchange(other.m_token, 0);
    }
    return *this;
}

void RadarDataSubscription::reset() {
    if (m_data) m_data->unsubscribe(m_token);
    m_data = nullptr;
    m_token = 0;
}

}  // namespace Sarwave::Core::Gui::ViewModels
