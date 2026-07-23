#include "rodavarion/runtime/ActionEngine.hpp"

#include "rodavarion/action/MouseMappingStore.hpp"
#include "rodavarion/safety/ActionSafetyPolicy.hpp"

#include <QFileInfo>

#include <utility>

namespace rodavarion::runtime {

ActionEngine::ActionEngine(IOutputEngine& outputEngine)
    : ActionEngine(
        outputEngine,
        action::MouseMappingStore::defaultFilePath()
    ) {}

ActionEngine::ActionEngine(
    IOutputEngine& outputEngine,
    QString mappingsFilePath
)
    : outputEngine_(outputEngine),
      mappingsFilePath_(std::move(mappingsFilePath)) {}

OutputResult ActionEngine::dispatch(
    const device::PeripheralDescriptor& peripheral,
    const input::InputEvent& event
) {
    if (event.kind == input::InputEventKind::Released) {
        return {.success = true, .message = "Подію відпускання пропущено."};
    }

    const auto button = mappingButton(event.control);
    if (!button.has_value()) {
        return {
            .success = false,
            .message = "Невідома подія пристрою відхилена."
        };
    }

    QString cacheError;
    if (!refreshCacheIfNeeded(
            peripheral.physicalDevice.key,
            cacheError
        )) {
        return {.success = false, .message = cacheError};
    }

    for (const auto& mapping : cachedProfile_->mappings) {
        if (mapping.button != *button) {
            continue;
        }

        if (mapping.action == action::ActionType::Default) {
            return {.success = true, .message = "Використано системну дію."};
        }

        if (mapping.action == action::ActionType::Disabled) {
            return {.success = true, .message = "Кнопку вимкнено."};
        }

        const auto safety =
            safety::ActionSafetyPolicy::validateForRuntime(mapping);
        if (!safety.allowed) {
            return {
                .success = false,
                .message = QString::fromStdString(safety.reason)
            };
        }

        return outputEngine_.execute(
            mapping.action,
            QString::fromStdString(mapping.parameter)
        );
    }

    return {.success = true, .message = "Відповідного призначення немає."};
}

bool ActionEngine::refreshCacheIfNeeded(
    const std::string& deviceKey,
    QString& errorMessage
) {
    const QFileInfo fileInfo(mappingsFilePath_);
    const bool exists = fileInfo.exists();
    const auto lastModified = exists ? fileInfo.lastModified() : QDateTime{};
    const qint64 fileSize = exists ? fileInfo.size() : -1;

    const bool cacheIsCurrent =
        cachedProfile_.has_value()
        && cachedDeviceKey_ == deviceKey
        && cachedFileExists_ == exists
        && cachedLastModified_ == lastModified
        && cachedFileSize_ == fileSize;

    if (cacheIsCurrent) {
        return true;
    }

    action::MouseMappingStore store;
    auto loaded = store.load(mappingsFilePath_, deviceKey);
    if (!loaded) {
        errorMessage = QString::fromStdString(loaded.error());
        return false;
    }

    cachedProfile_ = std::move(loaded.value());
    cachedDeviceKey_ = deviceKey;
    cachedFileExists_ = exists;
    cachedLastModified_ = lastModified;
    cachedFileSize_ = fileSize;
    return true;
}

std::optional<action::MouseButton> ActionEngine::mappingButton(
    const input::DeviceControl control
) noexcept {
    switch (control) {
        case input::DeviceControl::PrimaryButton: return action::MouseButton::Left;
        case input::DeviceControl::SecondaryButton: return action::MouseButton::Right;
        case input::DeviceControl::VerticalWheelClick: return action::MouseButton::Middle;
        case input::DeviceControl::ThumbButtonNearPalm: return action::MouseButton::Back;
        case input::DeviceControl::ThumbButtonMiddle: return action::MouseButton::Forward;
        case input::DeviceControl::ThumbButtonNearFront: return action::MouseButton::Extra1;
        case input::DeviceControl::HapticMenuButton: return action::MouseButton::Gesture;
        case input::DeviceControl::MagneticWheelMode: return action::MouseButton::Extra2;
        case input::DeviceControl::VerticalWheelUp: return action::MouseButton::WheelUp;
        case input::DeviceControl::VerticalWheelDown: return action::MouseButton::WheelDown;
        case input::DeviceControl::HorizontalWheelLeft: return action::MouseButton::WheelLeft;
        case input::DeviceControl::HorizontalWheelRight: return action::MouseButton::WheelRight;
        case input::DeviceControl::Unknown: return std::nullopt;
    }
    return std::nullopt;
}

} // namespace rodavarion::runtime
