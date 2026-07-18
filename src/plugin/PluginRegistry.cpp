#include "rodavarion/plugin/PluginRegistry.hpp"
#include "rodavarion/core/Logger.hpp"

#include <algorithm>
#include <string>

namespace rodavarion::plugin {

bool PluginRegistry::registerPlugin(std::unique_ptr<IPlugin> plugin) {
    if (!plugin) {
        return false;
    }

    const auto metadata = plugin->metadata();
    if (metadata.apiVersion != PluginApiVersion || metadata.id.empty()) {
        core::Logger::instance().log(
            core::LogLevel::Error,
            "Rejected plugin because metadata or API version is invalid."
        );
        return false;
    }

    const bool duplicate = std::any_of(
        plugins_.begin(),
        plugins_.end(),
        [metadata](const auto& existing) {
            return existing->metadata().id == metadata.id;
        }
    );

    if (duplicate) {
        return false;
    }

    plugins_.push_back(std::move(plugin));
    return true;
}

const std::vector<std::unique_ptr<IPlugin>>&
PluginRegistry::plugins() const noexcept {
    return plugins_;
}

} // namespace rodavarion::plugin
