#pragma once

#include "rodavarion/config/ConfigService.hpp"
#include "rodavarion/device/DeviceManager.hpp"
#include "rodavarion/device/PeripheralDescriptor.hpp"
#include "rodavarion/event/EventBus.hpp"
#include "rodavarion/plugin/PluginRegistry.hpp"
#include "rodavarion/profile/ProfileManager.hpp"
#include "rodavarion/profile/ProfileStore.hpp"

#include <vector>

namespace rodavarion::core {

class Application final {
public:
    Application();

    void initialize();

    [[nodiscard]] std::vector<device::PeripheralDescriptor>
        scanPeripherals();

    [[nodiscard]] bool saveProfiles();

    [[nodiscard]] profile::ProfileManager& profiles() noexcept;
    [[nodiscard]] event::EventBus& events() noexcept;

private:
    void ensureDefaultProfiles();

    event::EventBus eventBus_;
    config::ConfigService configService_;
    plugin::PluginRegistry pluginRegistry_;
    device::DeviceManager deviceManager_;
    profile::ProfileManager profileManager_;
    profile::ProfileStore profileStore_;
};

} // namespace rodavarion::core
