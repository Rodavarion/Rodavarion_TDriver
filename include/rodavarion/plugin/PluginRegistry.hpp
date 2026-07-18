#pragma once

#include "rodavarion/plugin/Plugin.hpp"

#include <memory>
#include <vector>

namespace rodavarion::plugin {

class PluginRegistry final {
public:
    bool registerPlugin(std::unique_ptr<IPlugin> plugin);
    [[nodiscard]] const std::vector<std::unique_ptr<IPlugin>>& plugins() const noexcept;

private:
    std::vector<std::unique_ptr<IPlugin>> plugins_;
};

} // namespace rodavarion::plugin
