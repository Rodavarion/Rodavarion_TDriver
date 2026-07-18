#pragma once

#include "rodavarion/device/Device.hpp"

#include <string>
#include <vector>

namespace rodavarion::device {

struct PhysicalDevice {
    std::string key;
    std::string manufacturer;
    std::string productName;
    std::vector<DeviceInfo> interfaces;

    [[nodiscard]] std::size_t interfaceCount() const noexcept {
        return interfaces.size();
    }

    [[nodiscard]] bool hasRealBackend() const noexcept {
        for (const auto& interfaceInfo : interfaces) {
            if (interfaceInfo.backendName != "Mock") {
                return true;
            }
        }

        return false;
    }
};

} // namespace rodavarion::device
