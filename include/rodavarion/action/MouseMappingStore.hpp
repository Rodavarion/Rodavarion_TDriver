#pragma once

#include "rodavarion/action/MouseAction.hpp"
#include "rodavarion/core/Result.hpp"

#include <QString>

namespace rodavarion::action {

class MouseMappingStore final {
public:
    [[nodiscard]] static QString defaultFilePath();

    [[nodiscard]] core::Result<MouseMappingProfile>
        load(const QString& filePath, const std::string& deviceKey) const;

    [[nodiscard]] core::Result<void>
        save(const QString& filePath, const MouseMappingProfile& profile) const;
};

} // namespace rodavarion::action
