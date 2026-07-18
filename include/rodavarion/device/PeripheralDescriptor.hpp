#pragma once

#include "rodavarion/capability/DeviceCapabilityClassifier.hpp"
#include "rodavarion/device/PhysicalDevice.hpp"
#include "rodavarion/transport/TransportType.hpp"

#include <vector>

namespace rodavarion::device {

struct PeripheralDescriptor {
    PhysicalDevice physicalDevice;
    capability::DeviceClassification classification;
    transport::TransportType transport{
        transport::TransportType::Unknown
    };
};

class PeripheralDescriptorFactory final {
public:
    [[nodiscard]] static PeripheralDescriptor
        create(const PhysicalDevice& device);
};

} // namespace rodavarion::device
