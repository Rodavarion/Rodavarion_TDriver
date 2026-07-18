#include "rodavarion/device/PeripheralDescriptor.hpp"
#include "rodavarion/transport/TransportClassifier.hpp"

namespace rodavarion::device {

PeripheralDescriptor
PeripheralDescriptorFactory::create(const PhysicalDevice& device) {
    return {
        .physicalDevice = device,
        .classification =
            capability::DeviceCapabilityClassifier::classify(device),
        .transport =
            transport::TransportClassifier::classify(device)
    };
}

} // namespace rodavarion::device
