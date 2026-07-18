#include "rodavarion/event/EventBus.hpp"

namespace rodavarion::event {

void EventBus::subscribe(std::string eventType, EventHandler handler) {
    std::lock_guard lock(mutex_);
    handlers_[std::move(eventType)].push_back(std::move(handler));
}

void EventBus::publish(const Event& event) const {
    std::vector<EventHandler> handlers;

    {
        std::lock_guard lock(mutex_);
        const auto it = handlers_.find(event.type);
        if (it == handlers_.end()) {
            return;
        }
        handlers = it->second;
    }

    for (const auto& handler : handlers) {
        handler(event);
    }
}

} // namespace rodavarion::event
