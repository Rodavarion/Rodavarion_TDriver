#pragma once

#include "rodavarion/device/PhysicalDevice.hpp"
#include "rodavarion/transport/TransportType.hpp"

namespace rodavarion::transport {

class TransportClassifier final {
public:
    [[nodiscard]] static TransportType
        classify(const device::PhysicalDevice& device);
};

} // namespace rodavarion::transport
