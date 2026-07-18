#include "rodavarion/runtime/ActionEngine.hpp"
#include "rodavarion/action/MouseMappingStore.hpp"
#include "rodavarion/safety/ActionSafetyPolicy.hpp"

namespace rodavarion::runtime {

ActionEngine::ActionEngine(IOutputEngine& outputEngine)
    : outputEngine_(outputEngine) {}

OutputResult ActionEngine::dispatch(
    const device::PeripheralDescriptor& peripheral,
    const input::InputEvent& event
) {
    if (event.kind == input::InputEventKind::Released) {
        return {.success = true, .message = "Подію відпускання пропущено."};
    }

    action::MouseMappingStore store;
    const auto loaded = store.load(
        action::MouseMappingStore::defaultFilePath(),
        peripheral.physicalDevice.key
    );

    if (!loaded) {
        return {
            .success = false,
            .message = QString::fromStdString(loaded.error())
        };
    }

    const auto button = mappingButton(event.control);
    for (const auto& mapping : loaded.value().mappings) {
        if (mapping.button != button) {
            continue;
        }

        if (mapping.action == action::ActionType::Default
            || mapping.action == action::ActionType::Disabled) {
            return {.success = true, .message = "Дію не призначено."};
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

action::MouseButton ActionEngine::mappingButton(
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
        case input::DeviceControl::Unknown: return action::MouseButton::Left;
    }
    return action::MouseButton::Left;
}

} // namespace rodavarion::runtime
