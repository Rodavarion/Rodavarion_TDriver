#include "rodavarion/mock/MockDeviceProvider.hpp"

namespace rodavarion::mock {

namespace {

class MockDevice final : public device::IDevice {
public:
    explicit MockDevice(device::DeviceInfo info)
        : info_(std::move(info)) {}

    [[nodiscard]] const device::DeviceInfo& info() const noexcept override {
        return info_;
    }

    [[nodiscard]] bool isConnected() const noexcept override {
        return opened_;
    }

    bool open() override {
        opened_ = true;
        info_.connected = true;
        return true;
    }

    void close() noexcept override {
        opened_ = false;
        info_.connected = false;
    }

private:
    device::DeviceInfo info_;
    bool opened_{false};
};

} // namespace

std::string_view MockDeviceProvider::name() const noexcept {
    return "Mock Device Provider";
}

std::vector<device::DeviceInfo> MockDeviceProvider::scan() {
    return {
        {
            .id = {0x046D, 0xB034, "MOCK-MX-001"},
            .manufacturer = "Mock Vendor",
            .productName = "Mock Advanced Mouse",
            .systemPath = "mock://mouse/1",
            .backendName = "Mock",
            .connection = device::ConnectionType::WirelessReceiver,
            .connected = true
        },
        {
            .id = {0x1209, 0x0001, "MOCK-KB-001"},
            .manufacturer = "Rodavarion Lab",
            .productName = "Virtual Test Keyboard",
            .systemPath = "mock://keyboard/1",
            .backendName = "Mock",
            .connection = device::ConnectionType::Virtual,
            .connected = true
        }
    };
}

std::unique_ptr<device::IDevice>
MockDeviceProvider::create(const device::DeviceId& id) {
    for (const auto& info : scan()) {
        if (info.id == id) {
            return std::make_unique<MockDevice>(info);
        }
    }
    return {};
}

} // namespace rodavarion::mock
