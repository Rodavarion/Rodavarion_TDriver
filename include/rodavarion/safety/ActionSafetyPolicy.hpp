#pragma once

#include "rodavarion/action/MouseAction.hpp"

#include <string>

namespace rodavarion::safety {

struct SafetyDecision {
    bool allowed{false};
    std::string reason;
};

class ActionSafetyPolicy final {
public:
    [[nodiscard]] static SafetyDecision
        validateForDryRun(const action::MouseButtonMapping& mapping);

    [[nodiscard]] static SafetyDecision
        validateForRuntime(const action::MouseButtonMapping& mapping);
};

} // namespace rodavarion::safety
