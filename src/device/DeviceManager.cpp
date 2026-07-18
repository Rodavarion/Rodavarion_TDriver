#include "rodavarion/device/DeviceManager.hpp"
#include "rodavarion/core/Logger.hpp"

#include <iterator>
#include <string>

namespace rodavarion::device {

DeviceManager::DeviceManager(event::EventBus& eventBus)
    : eventBus_(eventBus) {}

void DeviceManager::addProvider(std::unique_ptr<IDeviceProvider> provider) {
    if (!provider) {
        core::Logger::instance().log(
            core::LogLevel::Warning,
            "Attempted to add an empty device provider."
        );
        return;
    }

    core::Logger::instance().log(
        core::LogLevel::Info,
        std::string("Registered device provider: ") + std::string(provider->name())
    );

    providers_.push_back(std::move(provider));
}

std::vector<DeviceInfo> DeviceManager::scanAll() {
    std::vector<DeviceInfo> result;

    for (auto& provider : providers_) {
        auto devices = provider->scan();
        result.insert(
            result.end(),
            std::make_move_iterator(devices.begin()),
            std::make_move_iterator(devices.end())
        );
    }

    eventBus_.publish({
        .type = "devices.scanned",
        .payload = std::to_string(result.size())
    });

    return result;
}

} // namespace rodavarion::device
