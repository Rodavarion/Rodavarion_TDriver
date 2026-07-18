#pragma once
#include "rodavarion/device/PhysicalDevice.hpp"
#include <vector>
namespace rodavarion::device {
class PhysicalDeviceGrouper final {
public:
 [[nodiscard]] static std::vector<PhysicalDevice> group(const std::vector<DeviceInfo>& interfaces);
};
}
