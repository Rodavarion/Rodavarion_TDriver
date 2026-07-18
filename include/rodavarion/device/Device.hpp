#pragma once

#include <cstdint>
#include <string>

namespace rodavarion::device {

enum class ConnectionType {
    Unknown,
    Usb,
    Bluetooth,
    WirelessReceiver,
    Virtual
};

struct DeviceId {
    std::uint16_t vendorId{};
    std::uint16_t productId{};
    std::string serialNumber;

    [[nodiscard]] bool operator==(const DeviceId&) const = default;
};

struct DeviceInfo {
    DeviceId id;
    std::string manufacturer;
    std::string productName;
    std::string systemPath;
    std::string backendName;
    ConnectionType connection{ConnectionType::Unknown};
    bool connected{false};
};

class IDevice {
public:
    virtual ~IDevice() = default;

    [[nodiscard]] virtual const DeviceInfo& info() const noexcept = 0;
    [[nodiscard]] virtual bool isConnected() const noexcept = 0;

    virtual bool open() = 0;
    virtual void close() noexcept = 0;
};

} // namespace rodavarion::device
