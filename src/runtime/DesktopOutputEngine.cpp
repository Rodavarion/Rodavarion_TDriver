#include "rodavarion/runtime/DesktopOutputEngine.hpp"
#include "rodavarion/runtime/DesktopActionExecutor.hpp"

namespace rodavarion::runtime {

OutputResult DesktopOutputEngine::execute(
    const action::ActionType action,
    const QString& parameter
) {
    const auto result = DesktopActionExecutor::execute(action, parameter);
    return {.success = result.success, .message = result.message};
}

QString DesktopOutputEngine::description() const {
    return DesktopActionExecutor::backendDescription();
}

} // namespace rodavarion::runtime
