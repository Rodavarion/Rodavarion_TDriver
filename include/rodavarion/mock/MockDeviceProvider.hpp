#pragma once

#include "rodavarion/device/DeviceProvider.hpp"

namespace rodavarion::mock {

class MockDeviceProvider final : public device::IDeviceProvider {
public:
    [[nodiscard]] std::string_view name() const noexcept override;
    [[nodiscard]] std::vector<device::DeviceInfo> scan() override;
    [[nodiscard]] std::unique_ptr<device::IDevice>
        create(const device::DeviceId& id) override;
};

} // namespace rodavarion::mock
