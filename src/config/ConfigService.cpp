#include "rodavarion/config/ConfigService.hpp"

#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>

namespace rodavarion::config {

core::Result<void> ConfigService::load(const QString& filePath) {
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        return core::Result<void>::failure(
            QString("Cannot open configuration file: %1")
                .arg(filePath)
                .toStdString()
        );
    }

    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(file.readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return core::Result<void>::failure(
            QString("Invalid configuration file: %1")
                .arg(parseError.errorString())
                .toStdString()
        );
    }

    root_ = document.object();
    return core::Result<void>::success();
}

QString ConfigService::stringValue(
    const QString& section,
    const QString& key,
    const QString& fallback
) const {
    const auto sectionValue = root_.value(section);
    if (!sectionValue.isObject()) {
        return fallback;
    }

    return sectionValue.toObject().value(key).toString(fallback);
}

} // namespace rodavarion::config
