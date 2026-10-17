#include "Gui/Models/RadarData.hpp"

#include <algorithm>

namespace Sarwave::Core::Gui::Models {

RadarData::RadarData(MMWave::TimeSpan duration) : m_manager(duration) {
}

int RadarData::subscribe(Handler handler) {
    std::lock_guard lock(m_subscribersMutex);
    auto subscriber = std::make_shared<Subscriber>();
    subscriber->token = m_nextToken++;
    subscriber->handler = std::move(handler);
    m_subscribers.push_back(std::move(subscriber));
    return m_subscribers.back()->token;
}

void RadarData::unsubscribe(int token) {
    std::lock_guard raise(m_raiseMutex);
    std::lock_guard lock(m_subscribersMutex);
    const auto it = std::ranges::find_if(
        m_subscribers, [token](const auto& subscriber) { return subscriber->token == token; });
    if (it == m_subscribers.end()) return;

    (*it)->removed = true;
    m_subscribers.erase(it);
}

void RadarData::update(const MMWave::Streaming::Frame& frame, MMWave::TimeSpan timestamp) {
    update(frame, timestamp, MMWave::Quaternion::identity());
}

void RadarData::update(const MMWave::Streaming::Frame& frame, MMWave::TimeSpan timestamp,
                       MMWave::Quaternion leveling) {
    m_manager.update(frame, timestamp, leveling);
    onUpdated(timestamp);
}

void RadarData::skipFrame(MMWave::TimeSpan timestamp) {
    m_manager.skipFrame(timestamp);
    onUpdated(timestamp);
}

void RadarData::onUpdated(MMWave::TimeSpan timestamp) {
    m_lastUpdate = timestamp;
    raiseUpdated();
}

void RadarData::raiseUpdated() {
    std::lock_guard raise(m_raiseMutex);

    // Like a C# event: the handlers subscribed when the raise starts are the ones called, except
    // that one unsubscribed by an earlier handler of this same raise is skipped.
    std::vector<std::shared_ptr<Subscriber>> subscribers;
    {
        std::lock_guard lock(m_subscribersMutex);
        subscribers = m_subscribers;
    }

    for (const auto& subscriber : subscribers) {
        if (subscriber->removed) continue;
        subscriber->handler();
    }
}

}  // namespace Sarwave::Core::Gui::Models
