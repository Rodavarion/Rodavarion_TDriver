#include "rodavarion/action/MouseAction.hpp"

namespace rodavarion::action {

std::string_view mouseButtonName(const MouseButton button) noexcept {
    switch (button) {
        case MouseButton::Left:       return "Left button";
        case MouseButton::Right:      return "Right button";
        case MouseButton::Middle:     return "Middle button";
        case MouseButton::Back:       return "Back button";
        case MouseButton::Forward:    return "Forward button";
        case MouseButton::Gesture:    return "Gesture button";
        case MouseButton::WheelLeft:  return "Wheel left";
        case MouseButton::WheelRight: return "Wheel right";
        case MouseButton::Extra1:     return "Extra button 1";
        case MouseButton::Extra2:     return "Extra button 2";
        case MouseButton::WheelUp:    return "Wheel up";
        case MouseButton::WheelDown:  return "Wheel down";
    }
    return "Unknown button";
}

std::string_view actionTypeName(const ActionType action) noexcept {
    switch (action) {
        case ActionType::Default:             return "Default";
        case ActionType::Disabled:            return "Disabled";
        case ActionType::Copy:                return "Copy";
        case ActionType::Paste:               return "Paste";
        case ActionType::Cut:                 return "Cut";
        case ActionType::Undo:                return "Undo";
        case ActionType::Redo:                return "Redo";
        case ActionType::SelectAll:           return "Select all";
        case ActionType::Back:                return "Back";
        case ActionType::Forward:             return "Forward";
        case ActionType::ShowDesktop:         return "Show desktop";
        case ActionType::ApplicationSwitcher: return "Application switcher";
        case ActionType::VolumeUp:            return "Volume up";
        case ActionType::VolumeDown:          return "Volume down";
        case ActionType::Mute:                return "Mute";
        case ActionType::PlayPause:           return "Play / pause";
        case ActionType::CustomShortcut:      return "Custom shortcut";
        case ActionType::RunScript:           return "Run script";
    }
    return "Default";
}

std::vector<MouseButton> defaultMouseButtons() {
    return {
        MouseButton::Left,
        MouseButton::Right,
        MouseButton::Middle,
        MouseButton::Back,
        MouseButton::Forward,
        MouseButton::Gesture,
        MouseButton::WheelLeft,
        MouseButton::WheelRight,
        MouseButton::Extra1,
        MouseButton::Extra2,
        MouseButton::WheelUp,
        MouseButton::WheelDown
    };
}

std::vector<ActionType> availableMouseActions() {
    return {
        ActionType::Default,
        ActionType::Disabled,
        ActionType::Copy,
        ActionType::Paste,
        ActionType::Cut,
        ActionType::Undo,
        ActionType::Redo,
        ActionType::SelectAll,
        ActionType::Back,
        ActionType::Forward,
        ActionType::ShowDesktop,
        ActionType::ApplicationSwitcher,
        ActionType::VolumeUp,
        ActionType::VolumeDown,
        ActionType::Mute,
        ActionType::PlayPause,
        ActionType::CustomShortcut,
        ActionType::RunScript
    };
}

ActionType actionTypeFromName(const std::string_view name) noexcept {
    for (const auto action : availableMouseActions()) {
        if (actionTypeName(action) == name) {
            return action;
        }
    }
    return ActionType::Default;
}

} // namespace rodavarion::action
