#pragma once

#include "rodavarion/action/MouseAction.hpp"

#include <QString>

namespace rodavarion::runtime {

struct ExecutionResult {
    bool success{false};
    QString message;
};

class DesktopActionExecutor final {
public:
    [[nodiscard]] static ExecutionResult execute(
        action::ActionType action,
        const QString& parameter = {}
    );

    [[nodiscard]] static QString backendDescription();
    [[nodiscard]] static QString diagnosticReport();

private:
    [[nodiscard]] static ExecutionResult executeShortcut(
        const QString& portableShortcut
    );
};

} // namespace rodavarion::runtime
