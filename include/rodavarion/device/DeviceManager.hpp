#pragma once

#include "rodavarion/device/DeviceProvider.hpp"
#include "rodavarion/event/EventBus.hpp"

#include <memory>
#include <vector>

namespace rodavarion::device {

class DeviceManager final {
public:
    explicit DeviceManager(event::EventBus& eventBus);

    void addProvider(std::unique_ptr<IDeviceProvider> provider);
    [[nodiscard]] std::vector<DeviceInfo> scanAll();

private:
    event::EventBus& eventBus_;
    std::vector<std::unique_ptr<IDeviceProvider>> providers_;
};

} // namespace rodavarion::device
