#pragma once

#include "rodavarion/action/MouseAction.hpp"
#include "rodavarion/input/InputEvent.hpp"

namespace rodavarion::input {

class MouseEventNormalizer final {
public:
    [[nodiscard]] static InputEvent normalize(
        action::MouseButton button,
        bool pressed,
        int nativeCode
    ) noexcept;
};

} // namespace rodavarion::input
