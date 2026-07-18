#include "rodavarion/core/Application.hpp"

#include "rodavarion/core/Logger.hpp"
#include "rodavarion/device/PeripheralDescriptor.hpp"
#include "rodavarion/device/PhysicalDeviceGrouper.hpp"
#include "rodavarion/hid/HidDeviceProvider.hpp"
#include "rodavarion/mock/MockDeviceProvider.hpp"

#include <string>

namespace rodavarion::core {

Application::Application()
    : deviceManager_(eventBus_) {}

void Application::initialize() {
    Logger::instance().log(
        LogLevel::Info,
        std::string("Initializing Rodavarion TDriver ") + RODAVARION_VERSION_STRING + "."
    );

    const auto loaded = profileStore_.load(
        profile::ProfileStore::defaultFilePath()
    );

    if (loaded) {
        profileManager_.replaceAll(loaded.value());
    } else {
        Logger::instance().log(
            LogLevel::Warning,
            loaded.error()
        );
    }

    ensureDefaultProfiles();

    deviceManager_.addProvider(
        std::make_unique<hid::HidDeviceProvider>()
    );

#if RODAVARION_ENABLE_MOCK_DEVICES
    deviceManager_.addProvider(
        std::make_unique<mock::MockDeviceProvider>()
    );
#endif
}

void Application::ensureDefaultProfiles() {
    if (!profileManager_.all().empty()) {
        return;
    }

    profileManager_.add({
        .id = "default",
        .displayName = "Default profile",
        .settings = {
            {"pointer.speed", "1.0"},
            {"scroll.mode", "standard"}
        }
    });

    profileManager_.add({
        .id = "work",
        .displayName = "Work profile",
        .settings = {
            {"pointer.speed", "0.85"},
            {"scroll.mode", "smooth"}
        }
    });

    if (!saveProfiles()) {
        Logger::instance().log(
            LogLevel::Warning,
            "Unable to save the initial default profiles."
        );
    }
}

std::vector<device::PeripheralDescriptor>
Application::scanPeripherals() {
    const auto physicalDevices =
        device::PhysicalDeviceGrouper::group(
            deviceManager_.scanAll()
        );

    std::vector<device::PeripheralDescriptor> result;
    result.reserve(physicalDevices.size());

    for (const auto& physicalDevice : physicalDevices) {
        result.push_back(
            device::PeripheralDescriptorFactory::create(
                physicalDevice
            )
        );
    }

    return result;
}

bool Application::saveProfiles() {
    const auto result = profileStore_.save(
        profile::ProfileStore::defaultFilePath(),
        profileManager_.all()
    );

    if (!result) {
        Logger::instance().log(
            LogLevel::Error,
            result.error()
        );
        return false;
    }

    eventBus_.publish({
        .type = "profiles.saved",
        .payload = profile::ProfileStore::defaultFilePath().toStdString()
    });

    return true;
}

profile::ProfileManager& Application::profiles() noexcept {
    return profileManager_;
}

event::EventBus& Application::events() noexcept {
    return eventBus_;
}

} // namespace rodavarion::core
