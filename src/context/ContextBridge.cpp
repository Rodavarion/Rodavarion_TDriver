#include "rodavarion/context/ContextBridge.hpp"
#include "rodavarion/context/ApplicationContext.hpp"
#include "rodavarion/core/Logger.hpp"

namespace rodavarion::context {

ContextBridge::ContextBridge(QObject* parent) : QObject(parent) {}

void ContextBridge::SetActiveWindow(const QString& identity) {
    ApplicationContext::setActiveWindowIdentity(identity);
    core::Logger::instance().log(
        core::LogLevel::Debug,
        QString("Active application context: %1").arg(identity).toStdString()
    );
}

QString ContextBridge::GetActiveWindow() const {
    return ApplicationContext::activeProcessName();
}

} // namespace rodavarion::context
