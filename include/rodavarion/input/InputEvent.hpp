#pragma once

#include <chrono>

namespace rodavarion::input {

enum class DeviceControl {
    Unknown,
    PrimaryButton,
    SecondaryButton,
    VerticalWheelClick,
    ThumbButtonNearPalm,
    ThumbButtonMiddle,
    ThumbButtonNearFront,
    HapticMenuButton,
    MagneticWheelMode,
    VerticalWheelUp,
    VerticalWheelDown,
    HorizontalWheelLeft,
    HorizontalWheelRight
};

enum class InputEventKind {
    Pressed,
    Released,
    Step
};

struct InputEvent {
    DeviceControl control{DeviceControl::Unknown};
    InputEventKind kind{InputEventKind::Pressed};
    int nativeCode{0};
    int value{0};
    std::chrono::steady_clock::time_point timestamp{
        std::chrono::steady_clock::now()
    };
};

} // namespace rodavarion::input
