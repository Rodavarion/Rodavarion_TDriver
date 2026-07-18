#pragma once

#include "rodavarion/device/PeripheralDescriptor.hpp"
#include "rodavarion/input/InputEvent.hpp"
#include "rodavarion/runtime/IOutputEngine.hpp"
#include <QString>

namespace rodavarion::runtime {

class ActionEngine final {
public:
    explicit ActionEngine(IOutputEngine& outputEngine);

    [[nodiscard]] OutputResult dispatch(
        const device::PeripheralDescriptor& peripheral,
        const input::InputEvent& event
    );

private:
    [[nodiscard]] static action::MouseButton mappingButton(
        input::DeviceControl control
    ) noexcept;

    IOutputEngine& outputEngine_;
};

} // namespace rodavarion::runtime
