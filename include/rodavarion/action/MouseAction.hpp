#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace rodavarion::action {

enum class MouseButton {
    Left,
    Right,
    Middle,
    Back,
    Forward,
    Gesture,
    WheelLeft,
    WheelRight,
    Extra1,
    Extra2,
    WheelUp,
    WheelDown
};

enum class ActionType {
    Default,
    Disabled,
    Copy,
    Paste,
    Cut,
    Undo,
    Redo,
    SelectAll,
    Back,
    Forward,
    ShowDesktop,
    ApplicationSwitcher,
    VolumeUp,
    VolumeDown,
    Mute,
    PlayPause,
    CustomShortcut,
    RunScript
};

struct MouseButtonMapping {
    MouseButton button{MouseButton::Left};
    ActionType action{ActionType::Default};
    std::string parameter;
};

struct MouseMappingProfile {
    std::string deviceKey;
    std::vector<MouseButtonMapping> mappings;
};

[[nodiscard]] std::string_view mouseButtonName(MouseButton button) noexcept;
[[nodiscard]] std::string_view actionTypeName(ActionType action) noexcept;
[[nodiscard]] std::vector<MouseButton> defaultMouseButtons();
[[nodiscard]] std::vector<ActionType> availableMouseActions();
[[nodiscard]] ActionType actionTypeFromName(std::string_view name) noexcept;

} // namespace rodavarion::action
