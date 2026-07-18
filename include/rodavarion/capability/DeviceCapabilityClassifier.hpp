#pragma once

#include "rodavarion/capability/DeviceCapability.hpp"
#include "rodavarion/device/PhysicalDevice.hpp"

#include <vector>

namespace rodavarion::capability {

struct DeviceClassification {
    DeviceClass deviceClass{DeviceClass::Unknown};
    std::vector<DeviceCapability> capabilities;
};

class DeviceCapabilityClassifier final {
public:
    [[nodiscard]] static DeviceClassification
        classify(const device::PhysicalDevice& device);
};

} // namespace rodavarion::capability
