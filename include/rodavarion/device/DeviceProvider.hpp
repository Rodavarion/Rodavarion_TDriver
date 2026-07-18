#pragma once

#include "rodavarion/device/Device.hpp"

#include <memory>
#include <string_view>
#include <vector>

namespace rodavarion::device {

class IDeviceProvider {
public:
    virtual ~IDeviceProvider() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    [[nodiscard]] virtual std::vector<DeviceInfo> scan() = 0;
    [[nodiscard]] virtual std::unique_ptr<IDevice> create(const DeviceId& id) = 0;
};

} // namespace rodavarion::device
