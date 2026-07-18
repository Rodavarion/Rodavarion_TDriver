#pragma once

#include "rodavarion/action/MouseAction.hpp"
#include <QString>

namespace rodavarion::runtime {

struct OutputResult {
    bool success{false};
    QString message;
};

class IOutputEngine {
public:
    virtual ~IOutputEngine() = default;
    [[nodiscard]] virtual OutputResult execute(
        action::ActionType action,
        const QString& parameter
    ) = 0;
    [[nodiscard]] virtual QString description() const = 0;
};

} // namespace rodavarion::runtime
