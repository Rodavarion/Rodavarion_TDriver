#pragma once

#include <string_view>

namespace rodavarion::transport {

enum class TransportType {
    Unknown,
    Hid,
    Usb,
    Bluetooth,
    Serial,
    Network,
    Cups,
    Sane,
    Evdev,
    Virtual
};

[[nodiscard]] constexpr std::string_view
transportName(const TransportType value) noexcept {
    switch (value) {
        case TransportType::Hid:       return "HID";
        case TransportType::Usb:       return "USB";
        case TransportType::Bluetooth: return "Bluetooth";
        case TransportType::Serial:    return "Serial";
        case TransportType::Network:   return "Network";
        case TransportType::Cups:      return "CUPS";
        case TransportType::Sane:      return "SANE";
        case TransportType::Evdev:     return "evdev";
        case TransportType::Virtual:   return "Virtual";
        case TransportType::Unknown:   return "Unknown";
    }
    return "Unknown";
}

} // namespace rodavarion::transport
