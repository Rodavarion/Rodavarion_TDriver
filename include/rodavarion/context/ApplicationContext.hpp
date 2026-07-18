#pragma once

#include "rodavarion/action/MouseAction.hpp"
#include "rodavarion/core/Result.hpp"

#include <QString>
#include <QStringList>
#include <QVector>

namespace rodavarion::context {

struct ApplicationProfile {
    QString id;
    QString displayName;
    QStringList processPatterns;
    QString copyShortcut{"CTRL+C"};
    QString pasteShortcut{"CTRL+V"};
    QString cutShortcut{"CTRL+X"};
    QString undoShortcut{"CTRL+Z"};
    QString redoShortcut{"CTRL+SHIFT+Z"};
    QString selectAllShortcut{"CTRL+A"};
    bool enabled{true};
};

class ApplicationContext final {
public:
    [[nodiscard]] static QString defaultFilePath();
    [[nodiscard]] static QVector<ApplicationProfile> defaultProfiles();
    [[nodiscard]] static core::Result<QVector<ApplicationProfile>> load();
    [[nodiscard]] static core::Result<void> save(const QVector<ApplicationProfile>& profiles);

    static void setActiveWindowIdentity(const QString& identity);
    [[nodiscard]] static QString activeProcessName();
    [[nodiscard]] static QString shortcutFor(action::ActionType action);
    [[nodiscard]] static QString diagnosticReport();
};

} // namespace rodavarion::context
