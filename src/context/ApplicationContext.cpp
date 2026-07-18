#include "rodavarion/context/ApplicationContext.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSaveFile>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QMutex>
#include <QMutexLocker>

namespace rodavarion::context {
namespace {

QMutex activeIdentityMutex;
QString activeIdentity;

QString runAndRead(const QString& program, const QStringList& arguments) {
    const auto executable = QStandardPaths::findExecutable(program);
    if (executable.isEmpty()) return {};
    QProcess process;
    process.start(executable, arguments);
    if (!process.waitForFinished(800) || process.exitCode() != 0) {
        process.kill();
        return {};
    }
    return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

QString processNameForPid(const QString& pid) {
    bool ok = false;
    const auto numericPid = pid.trimmed().toLongLong(&ok);
    if (!ok || numericPid <= 0) return {};
    QFile file(QString("/proc/%1/comm").arg(numericPid));
    if (!file.open(QIODevice::ReadOnly)) return {};
    return QString::fromUtf8(file.readAll()).trimmed().toLower();
}


QString activeWindowIdentityWith(const QString& tool) {
    if (QStandardPaths::findExecutable(tool).isEmpty()) return {};

    const QStringList prefix = {"getactivewindow"};
    const QString pid = runAndRead(tool, prefix + QStringList{"getwindowpid"});
    const QString process = processNameForPid(pid);
    const QString windowClass = runAndRead(tool, prefix + QStringList{"getwindowclassname"}).toLower();
    const QString windowName = runAndRead(tool, prefix + QStringList{"getwindowname"}).toLower();

    QStringList parts;
    for (const auto& value : {process, windowClass, windowName}) {
        const auto normalized = value.trimmed().toLower();
        if (!normalized.isEmpty() && !parts.contains(normalized)) parts.append(normalized);
    }
    return parts.join(" | ");
}

QString shortcutFromProfile(const ApplicationProfile& profile, action::ActionType action) {
    switch (action) {
        case action::ActionType::Copy: return profile.copyShortcut;
        case action::ActionType::Paste: return profile.pasteShortcut;
        case action::ActionType::Cut: return profile.cutShortcut;
        case action::ActionType::Undo: return profile.undoShortcut;
        case action::ActionType::Redo: return profile.redoShortcut;
        case action::ActionType::SelectAll: return profile.selectAllShortcut;
        default: return {};
    }
}

QJsonObject toJson(const ApplicationProfile& profile) {
    QJsonArray patterns;
    for (const auto& pattern : profile.processPatterns) patterns.append(pattern);
    return {
        {"id", profile.id}, {"displayName", profile.displayName},
        {"processPatterns", patterns}, {"copy", profile.copyShortcut},
        {"paste", profile.pasteShortcut}, {"cut", profile.cutShortcut},
        {"undo", profile.undoShortcut}, {"redo", profile.redoShortcut},
        {"selectAll", profile.selectAllShortcut}, {"enabled", profile.enabled}
    };
}

ApplicationProfile fromJson(const QJsonObject& object) {
    ApplicationProfile profile;
    profile.id = object.value("id").toString();
    profile.displayName = object.value("displayName").toString(profile.id);
    for (const auto& value : object.value("processPatterns").toArray())
        profile.processPatterns.append(value.toString().toLower());
    profile.copyShortcut = object.value("copy").toString("CTRL+C");
    profile.pasteShortcut = object.value("paste").toString("CTRL+V");
    profile.cutShortcut = object.value("cut").toString("CTRL+X");
    profile.undoShortcut = object.value("undo").toString("CTRL+Z");
    profile.redoShortcut = object.value("redo").toString("CTRL+SHIFT+Z");
    profile.selectAllShortcut = object.value("selectAll").toString("CTRL+A");
    profile.enabled = object.value("enabled").toBool(true);
    return profile;
}

} // namespace

QString ApplicationContext::defaultFilePath() {
    const auto overrideDirectory = qEnvironmentVariable("RODAVARION_CONFIG_DIR").trimmed();
    const auto root = overrideDirectory.isEmpty()
        ? QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
        : overrideDirectory;
    const auto directory = QDir(root).filePath("Rodavarion Technologies/Rodavarion TDriver");
    QDir().mkpath(directory);
    return QDir(directory).filePath("application-contexts.json");
}

QVector<ApplicationProfile> ApplicationContext::defaultProfiles() {
    return {
        {"terminal", "Термінали", {"konsole", "org.kde.konsole", "gnome-terminal", "org.gnome.console", "kgx", "xfce4-terminal", "tilix", "kitty", "alacritty", "wezterm", "foot", "xterm", "terminator"}, "CTRL+SHIFT+C", "CTRL+SHIFT+V", "CTRL+SHIFT+X", "CTRL+SHIFT+Z", "CTRL+SHIFT+Z", "CTRL+SHIFT+A", true},
        {"browser", "Веббраузери", {"firefox", "org.mozilla.firefox", "chromium", "google-chrome", "google-chrome-stable", "brave", "brave-browser", "vivaldi", "opera", "microsoft-edge"}, "CTRL+C", "CTRL+V", "CTRL+X", "CTRL+Z", "CTRL+SHIFT+Z", "CTRL+A", true},
        {"vscode", "Visual Studio Code / VSCodium", {"code", "code-oss", "codium", "vscodium"}, "CTRL+C", "CTRL+V", "CTRL+X", "CTRL+Z", "CTRL+SHIFT+Z", "CTRL+A", true},
        {"jetbrains", "JetBrains IDE", {"idea", "idea64", "clion", "pycharm", "webstorm", "phpstorm", "rider", "goland", "rubymine", "datagrip", "android-studio"}, "CTRL+C", "CTRL+V", "CTRL+X", "CTRL+Z", "CTRL+SHIFT+Z", "CTRL+A", true},
        {"qtcreator", "Qt Creator", {"qtcreator"}, "CTRL+C", "CTRL+V", "CTRL+X", "CTRL+Z", "CTRL+Y", "CTRL+A", true},
        {"visualstudio", "Visual Studio", {"devenv"}, "CTRL+C", "CTRL+V", "CTRL+X", "CTRL+Z", "CTRL+Y", "CTRL+A", true},
        {"eclipse", "Eclipse", {"eclipse"}, "CTRL+C", "CTRL+V", "CTRL+X", "CTRL+Z", "CTRL+Y", "CTRL+A", true},
        {"neovim", "Neovim / Vim GUI", {"nvim-qt", "gvim", "neovide"}, "CTRL+C", "CTRL+V", "CTRL+X", "U", "CTRL+R", "CTRL+A", true},
        {"default", "Інші програми", {}, "CTRL+C", "CTRL+V", "CTRL+X", "CTRL+Z", "CTRL+SHIFT+Z", "CTRL+A", true}
    };
}

core::Result<QVector<ApplicationProfile>> ApplicationContext::load() {
    const auto canonicalPath = defaultFilePath();
    const auto overrideDirectory = qEnvironmentVariable("RODAVARION_CONFIG_DIR").trimmed();

    // One-time migration from builds where GUI and daemon used different
    // QStandardPaths directories. Never import real user data into an explicit
    // test/portable override directory.
    if (overrideDirectory.isEmpty() && !QFileInfo::exists(canonicalPath)) {
        const auto configRoot = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
        const QStringList legacyPaths = {
            QDir(configRoot).filePath("Rodavarion Technologies/Rodavarion TDriver Service/application-contexts.json"),
            QDir(configRoot).filePath("Rodavarion Technologies/Rodavarion TDriver/application-contexts.json")
        };
        for (const auto& legacyPath : legacyPaths) {
            if (legacyPath == canonicalPath || !QFileInfo::exists(legacyPath)) continue;
            QDir().mkpath(QFileInfo(canonicalPath).absolutePath());
            if (QFile::copy(legacyPath, canonicalPath)) break;
        }
    }

    QFile file(canonicalPath);
    if (!file.exists()) return core::Result<QVector<ApplicationProfile>>::success(defaultProfiles());
    if (!file.open(QIODevice::ReadOnly)) return core::Result<QVector<ApplicationProfile>>::failure("Cannot open application context settings.");
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !document.isArray())
        return core::Result<QVector<ApplicationProfile>>::failure("Invalid application context settings.");

    QVector<ApplicationProfile> profiles;
    for (const auto& value : document.array()) profiles.append(fromJson(value.toObject()));

    // Configuration schema migration: preserve every existing/user-edited
    // profile, but append system profiles introduced by newer releases. This is
    // essential for upgrades because an old JSON file otherwise masks new
    // browser/default profiles forever.
    bool changed = false;
    const auto builtIns = defaultProfiles();
    for (const auto& builtIn : builtIns) {
        bool exists = false;
        for (const auto& profile : profiles) {
            if (profile.id.compare(builtIn.id, Qt::CaseInsensitive) == 0) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            profiles.append(builtIn);
            changed = true;
        }
    }

    if (changed) {
        const auto saved = save(profiles);
        if (!saved) {
            return core::Result<QVector<ApplicationProfile>>::failure(
                "Cannot migrate application context settings: " + saved.error()
            );
        }
    }

    return core::Result<QVector<ApplicationProfile>>::success(std::move(profiles));
}

core::Result<void> ApplicationContext::save(const QVector<ApplicationProfile>& profiles) {
    QJsonArray array;
    for (const auto& profile : profiles) array.append(toJson(profile));
    QSaveFile file(defaultFilePath());
    if (!file.open(QIODevice::WriteOnly)) return core::Result<void>::failure("Cannot write application context settings.");
    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
    if (!file.commit()) return core::Result<void>::failure("Cannot commit application context settings.");
    return core::Result<void>::success();
}

void ApplicationContext::setActiveWindowIdentity(const QString& identity) {
    QMutexLocker lock(&activeIdentityMutex);
    activeIdentity = identity.trimmed().toLower();
}

QString ApplicationContext::activeProcessName() {
    const auto overrideName = qEnvironmentVariable("RODAVARION_ACTIVE_APP").trimmed().toLower();
    if (!overrideName.isEmpty()) return overrideName;

    {
        QMutexLocker lock(&activeIdentityMutex);
        if (!activeIdentity.isEmpty()) return activeIdentity;
    }

    // Optional compatibility providers. KDE Plasma 6 uses the bundled KWin
    // script and D-Bus bridge, so no external utility is required there.
    auto identity = activeWindowIdentityWith("kdotool");
    if (!identity.isEmpty()) return identity;
    return activeWindowIdentityWith("xdotool");
}

QString ApplicationContext::shortcutFor(action::ActionType action) {
    const auto processName = activeProcessName();
    if (processName.isEmpty()) return {};
    const auto loaded = load();
    if (!loaded) return {};
    const ApplicationProfile* fallbackProfile = nullptr;
    for (const auto& profile : loaded.value()) {
        if (!profile.enabled) continue;
        if (profile.id.compare("default", Qt::CaseInsensitive) == 0) {
            fallbackProfile = &profile;
            continue;
        }
        for (const auto& pattern : profile.processPatterns) {
            if (!pattern.isEmpty() && processName.contains(pattern, Qt::CaseInsensitive))
                return shortcutFromProfile(profile, action);
        }
    }
    return fallbackProfile != nullptr
        ? shortcutFromProfile(*fallbackProfile, action)
        : QString{};
}

QString ApplicationContext::diagnosticReport() {
    const auto active = activeProcessName();
    QString matchedProfile = "Інші програми";
    const auto loaded = load();
    if (loaded && !active.isEmpty()) {
        for (const auto& profile : loaded.value()) {
            if (!profile.enabled) continue;
            bool matched = false;
            for (const auto& pattern : profile.processPatterns) {
                if (!pattern.trimmed().isEmpty() && active.contains(pattern.trimmed(), Qt::CaseInsensitive)) {
                    matched = true;
                    break;
                }
            }
            if (matched) {
                matchedProfile = profile.displayName;
                break;
            }
        }
    }

    const auto copyShortcut = shortcutFor(action::ActionType::Copy);
    const auto pasteShortcut = shortcutFor(action::ActionType::Paste);
    return QString(
        "Активне вікно: %1\n"
        "Застосований профіль: %2\n"
        "Копіювати / вставити: %3 / %4\n"
        "Спільний файл профілів: %5\n"
        "Детектор: %6"
    )
        .arg(active.isEmpty() ? "не визначено" : active)
        .arg(matchedProfile)
        .arg(copyShortcut.isEmpty() ? "CTRL+C" : copyShortcut)
        .arg(pasteShortcut.isEmpty() ? "CTRL+V" : pasteShortcut)
        .arg(defaultFilePath())
        .arg(!active.isEmpty()
            ? "KWin D-Bus bridge / compatibility provider"
            : "контекст не отримано — безпечна дія не виконується");
}

} // namespace rodavarion::context
