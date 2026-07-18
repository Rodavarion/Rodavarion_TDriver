#pragma once

#include "rodavarion/device/DeviceProvider.hpp"

namespace rodavarion::hid {

class HidDeviceProvider final : public device::IDeviceProvider {
public:
    HidDeviceProvider();
    ~HidDeviceProvider() override;

    [[nodiscard]] std::string_view name() const noexcept override;
    [[nodiscard]] std::vector<device::DeviceInfo> scan() override;
    [[nodiscard]] std::unique_ptr<device::IDevice>
        create(const device::DeviceId& id) override;

private:
    bool initialized_{false};
};

} // namespace rodavarion::hid
