#pragma once

#include "rodavarion/device/DeviceProvider.hpp"

#include <memory>
#include <string_view>

namespace rodavarion::plugin {

inline constexpr int PluginApiVersion = 1;

struct PluginMetadata {
    std::string_view id;
    std::string_view name;
    std::string_view version;
    int apiVersion{PluginApiVersion};
};

class IPlugin {
public:
    virtual ~IPlugin() = default;

    [[nodiscard]] virtual PluginMetadata metadata() const noexcept = 0;
    [[nodiscard]] virtual std::unique_ptr<device::IDeviceProvider>
        createDeviceProvider() = 0;
};

} // namespace rodavarion::plugin
