#pragma once
#include "rodavarion/runtime/IOutputEngine.hpp"

namespace rodavarion::runtime {

class DesktopOutputEngine final : public IOutputEngine {
public:
    [[nodiscard]] OutputResult execute(
        action::ActionType action,
        const QString& parameter
    ) override;
    [[nodiscard]] QString description() const override;
};

} // namespace rodavarion::runtime
