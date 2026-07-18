#include "rodavarion/input/MouseEventNormalizer.hpp"

namespace rodavarion::input {

InputEvent MouseEventNormalizer::normalize(
    const action::MouseButton button,
    const bool pressed,
    const int nativeCode
) noexcept {
    DeviceControl control = DeviceControl::Unknown;
    InputEventKind kind = pressed
        ? InputEventKind::Pressed
        : InputEventKind::Released;

    switch (button) {
        case action::MouseButton::Left:
            control = DeviceControl::PrimaryButton;
            break;
        case action::MouseButton::Right:
            control = DeviceControl::SecondaryButton;
            break;
        case action::MouseButton::Middle:
            control = DeviceControl::VerticalWheelClick;
            break;
        case action::MouseButton::Back:
            control = DeviceControl::ThumbButtonNearPalm;
            break;
        case action::MouseButton::Forward:
            control = DeviceControl::ThumbButtonMiddle;
            break;
        case action::MouseButton::Gesture:
            control = DeviceControl::HapticMenuButton;
            break;
        case action::MouseButton::Extra1:
            control = DeviceControl::ThumbButtonNearFront;
            break;
        case action::MouseButton::Extra2:
            control = DeviceControl::MagneticWheelMode;
            break;
        case action::MouseButton::WheelUp:
            control = DeviceControl::VerticalWheelUp;
            kind = InputEventKind::Step;
            break;
        case action::MouseButton::WheelDown:
            control = DeviceControl::VerticalWheelDown;
            kind = InputEventKind::Step;
            break;
        case action::MouseButton::WheelLeft:
            control = DeviceControl::HorizontalWheelLeft;
            kind = InputEventKind::Step;
            break;
        case action::MouseButton::WheelRight:
            control = DeviceControl::HorizontalWheelRight;
            kind = InputEventKind::Step;
            break;
    }

    return {
        .control = control,
        .kind = kind,
        .nativeCode = nativeCode,
        .value = pressed ? 1 : 0
    };
}

} // namespace rodavarion::input
