#include "rodavarion/action/MouseMappingStore.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

namespace rodavarion::action {

namespace {

MouseButton buttonFromIndex(const int index) {
    const auto buttons = defaultMouseButtons();
    if (index < 0 || index >= static_cast<int>(buttons.size())) {
        return MouseButton::Left;
    }
    return buttons[static_cast<std::size_t>(index)];
}

} // namespace

QString MouseMappingStore::defaultFilePath() {
    const auto directory = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation
    );
    QDir().mkpath(directory);
    return QDir(directory).filePath("mouse-mappings.json");
}

core::Result<MouseMappingProfile>
MouseMappingStore::load(
    const QString& filePath,
    const std::string& deviceKey
) const {
    MouseMappingProfile result;
    result.deviceKey = deviceKey;

    for (const auto button : defaultMouseButtons()) {
        result.mappings.push_back({
            .button = button,
            .action = ActionType::Default,
            .parameter = {}
        });
    }

    QFile file(filePath);
    if (!file.exists()) {
        return core::Result<MouseMappingProfile>::success(
            std::move(result)
        );
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return core::Result<MouseMappingProfile>::failure(
            QString("Cannot open mouse mappings: %1")
                .arg(filePath)
                .toStdString()
        );
    }

    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(
        file.readAll(),
        &error
    );

    if (error.error != QJsonParseError::NoError
        || !document.isArray()) {
        return core::Result<MouseMappingProfile>::failure(
            "Invalid mouse mappings file."
        );
    }

    const auto profiles = document.array();
    QJsonObject selectedProfile;

    // Prefer the exact stable device key. Some HID stacks expose the same
    // physical mouse through slightly different interface keys in the GUI and
    // daemon. When there is exactly one saved mouse profile, use it as a safe
    // compatibility fallback instead of silently reverting to Default actions.
    for (const auto& profileValue : profiles) {
        const auto profileObject = profileValue.toObject();
        if (profileObject.value("deviceKey").toString().toStdString()
            == deviceKey) {
            selectedProfile = profileObject;
            break;
        }
    }

    if (selectedProfile.isEmpty() && profiles.size() == 1) {
        selectedProfile = profiles.first().toObject();
    }

    if (!selectedProfile.isEmpty()) {
        result.mappings.clear();
        const auto mappings = selectedProfile.value("mappings").toArray();
        for (const auto& mappingValue : mappings) {
            const auto object = mappingValue.toObject();
            result.mappings.push_back({
                .button = buttonFromIndex(object.value("button").toInt()),
                .action = actionTypeFromName(
                    object.value("action").toString().toStdString()
                ),
                .parameter = object.value("parameter").toString().toStdString()
            });
        }
    }

    return core::Result<MouseMappingProfile>::success(
        std::move(result)
    );
}

core::Result<void>
MouseMappingStore::save(
    const QString& filePath,
    const MouseMappingProfile& profile
) const {
    QJsonArray profiles;

    QFile existing(filePath);
    if (existing.exists() && existing.open(QIODevice::ReadOnly)) {
        QJsonParseError error;
        const auto document = QJsonDocument::fromJson(
            existing.readAll(),
            &error
        );
        if (error.error == QJsonParseError::NoError
            && document.isArray()) {
            profiles = document.array();
        }
    }

    QJsonArray filtered;
    for (const auto& value : profiles) {
        if (value.toObject().value("deviceKey").toString().toStdString()
            != profile.deviceKey) {
            filtered.append(value);
        }
    }

    QJsonArray mappings;
    const auto buttons = defaultMouseButtons();

    for (const auto& mapping : profile.mappings) {
        int buttonIndex = 0;
        for (int index = 0; index < static_cast<int>(buttons.size()); ++index) {
            if (buttons[static_cast<std::size_t>(index)] == mapping.button) {
                buttonIndex = index;
                break;
            }
        }

        QJsonObject object;
        object.insert("button", buttonIndex);
        object.insert(
            "action",
            QString::fromUtf8(actionTypeName(mapping.action).data())
        );
        object.insert(
            "parameter",
            QString::fromStdString(mapping.parameter)
        );
        mappings.append(object);
    }

    QJsonObject profileObject;
    profileObject.insert(
        "deviceKey",
        QString::fromStdString(profile.deviceKey)
    );
    profileObject.insert("mappings", mappings);
    filtered.append(profileObject);

    const QFileInfo info(filePath);
    QDir().mkpath(info.absolutePath());

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return core::Result<void>::failure(
            "Cannot write mouse mappings file."
        );
    }

    file.write(
        QJsonDocument(filtered).toJson(QJsonDocument::Indented)
    );

    if (!file.commit()) {
        return core::Result<void>::failure(
            "Cannot commit mouse mappings file."
        );
    }

    return core::Result<void>::success();
}

} // namespace rodavarion::action
