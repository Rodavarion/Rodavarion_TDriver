#include "rodavarion/transport/TransportClassifier.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace rodavarion::transport {

namespace {

std::string lowerCopy(std::string value) {
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](const unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        }
    );
    return value;
}

bool contains(const std::string& text, const std::string& value) {
    return text.find(value) != std::string::npos;
}

} // namespace

TransportType
TransportClassifier::classify(const device::PhysicalDevice& device) {
    for (const auto& interfaceInfo : device.interfaces) {
        const auto backend = lowerCopy(interfaceInfo.backendName);
        const auto path = lowerCopy(interfaceInfo.systemPath);

        if (contains(backend, "mock")) {
            return TransportType::Virtual;
        }
        if (contains(backend, "hid")) {
            return TransportType::Hid;
        }
        if (contains(path, "tty")) {
            return TransportType::Serial;
        }
        if (contains(path, "bluetooth")) {
            return TransportType::Bluetooth;
        }
    }

    return TransportType::Unknown;
}

} // namespace rodavarion::transport
