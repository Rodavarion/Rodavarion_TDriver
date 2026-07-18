#pragma once

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace rodavarion::event {

struct Event {
    std::string type;
    std::string payload;
};

using EventHandler = std::function<void(const Event&)>;

class EventBus final {
public:
    void subscribe(std::string eventType, EventHandler handler);
    void publish(const Event& event) const;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::vector<EventHandler>> handlers_;
};

} // namespace rodavarion::event
