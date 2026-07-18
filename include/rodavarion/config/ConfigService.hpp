#pragma once

#include "rodavarion/core/Result.hpp"

#include <QJsonObject>
#include <QString>

namespace rodavarion::config {

class ConfigService final {
public:
    [[nodiscard]] core::Result<void> load(const QString& filePath);
    [[nodiscard]] QString stringValue(
        const QString& section,
        const QString& key,
        const QString& fallback = {}
    ) const;

private:
    QJsonObject root_;
};

} // namespace rodavarion::config
