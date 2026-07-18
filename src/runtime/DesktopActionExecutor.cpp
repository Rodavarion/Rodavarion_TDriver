#include "rodavarion/runtime/DesktopActionExecutor.hpp"
#include "rodavarion/context/ApplicationContext.hpp"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QStringList>

#include <cerrno>
#include <chrono>
#include <cstring>
#include <cstdio>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <mutex>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>

namespace rodavarion::runtime {

namespace {

QString shortcutForAction(const action::ActionType action) {
    switch (action) {
        case action::ActionType::Copy:                return "CTRL+C";
        case action::ActionType::Paste:               return "CTRL+V";
        case action::ActionType::Cut:                 return "CTRL+X";
        case action::ActionType::Undo:                return "CTRL+Z";
        case action::ActionType::Redo:                return "CTRL+SHIFT+Z";
        case action::ActionType::SelectAll:           return "CTRL+A";
        case action::ActionType::Back:                return "ALT+LEFT";
        case action::ActionType::Forward:             return "ALT+RIGHT";
        case action::ActionType::ShowDesktop:         return "META+D";
        case action::ActionType::ApplicationSwitcher: return "ALT+TAB";
        case action::ActionType::VolumeUp:            return "VOLUMEUP";
        case action::ActionType::VolumeDown:          return "VOLUMEDOWN";
        case action::ActionType::Mute:                return "MUTE";
        case action::ActionType::PlayPause:           return "PLAYPAUSE";
        default:                                      return {};
    }
}

QStringList shortcutParts(QString shortcut) {
    shortcut = shortcut.trimmed().toUpper();
    return shortcut.split('+', Qt::SkipEmptyParts);
}

int linuxKeyCode(const QString& name) {
    if (name == "CTRL" || name == "CONTROL") return 29;
    if (name == "SHIFT") return 42;
    if (name == "ALT") return 56;
    if (name == "META" || name == "SUPER") return 125;
    if (name == "TAB") return 15;
    if (name == "LEFT") return 105;
    if (name == "RIGHT") return 106;
    if (name == "A") return 30;
    if (name == "C") return 46;
    if (name == "D") return 32;
    if (name == "V") return 47;
    if (name == "X") return 45;
    if (name == "Z") return 44;
    if (name == "VOLUMEUP") return 115;
    if (name == "VOLUMEDOWN") return 114;
    if (name == "MUTE") return 113;
    if (name == "PLAYPAUSE") return 164;
    return -1;
}


class DirectKeyboardDevice final {
public:
    DirectKeyboardDevice(const DirectKeyboardDevice&) = delete;
    DirectKeyboardDevice& operator=(const DirectKeyboardDevice&) = delete;

    static DirectKeyboardDevice& instance() {
        static DirectKeyboardDevice device;
        return device;
    }

    ExecutionResult sendShortcut(const QString& shortcut) {
        std::scoped_lock lock(mutex_);

        const auto parts = shortcutParts(shortcut);
        if (parts.isEmpty()) {
            return {false, "Комбінація клавіш порожня."};
        }

        QList<int> modifiers;
        int keyCode = -1;
        for (const auto& part : parts) {
            const int code = linuxKeyCode(part);
            if (code < 0) {
                return {false, QString("Непідтримувана клавіша у комбінації: %1").arg(part)};
            }
            if (part == "CTRL" || part == "CONTROL" || part == "SHIFT"
                || part == "ALT" || part == "META" || part == "SUPER") {
                modifiers.push_back(code);
            } else {
                keyCode = code;
            }
        }
        if (keyCode < 0) {
            return {false, "У комбінації відсутня основна клавіша."};
        }

        const auto ready = ensureReady();
        if (!ready.success) {
            return ready;
        }

        // Emit complete keyboard states in deterministic frames. Sending a
        // SYN_REPORT after every individual modifier caused some compositors
        // to observe Ctrl without Shift (or the reverse), producing intermittent
        // plain Ctrl+V. Each phase is now one atomic input frame.
        QList<QPair<int, int>> modifierPresses;
        for (const int modifier : modifiers) {
            modifierPresses.push_back({modifier, 1});
        }
        if (!emitFrame(modifierPresses)) {
            releaseAll(modifiers, keyCode);
            return lastWriteError();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(8));

        if (!emitFrame({{keyCode, 1}})) {
            releaseAll(modifiers, keyCode);
            return lastWriteError();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(12));

        if (!emitFrame({{keyCode, 0}})) {
            releaseAll(modifiers, keyCode);
            return lastWriteError();
        }

        QList<QPair<int, int>> modifierReleases;
        for (auto it = modifiers.crbegin(); it != modifiers.crend(); ++it) {
            modifierReleases.push_back({*it, 0});
        }
        if (!emitFrame(modifierReleases)) {
            return lastWriteError();
        }

        return {
            true,
            QString("Комбінацію %1 передано напряму через /dev/uinput.").arg(shortcut)
        };
    }

    bool available() {
        std::scoped_lock lock(mutex_);
        return ensureReady().success;
    }

    QString status() {
        std::scoped_lock lock(mutex_);
        const auto result = ensureReady();
        return result.success ? "готовий" : result.message;
    }

private:
    DirectKeyboardDevice() = default;
    ~DirectKeyboardDevice() {
        if (fd_ >= 0) {
            ::ioctl(fd_, UI_DEV_DESTROY);
            ::close(fd_);
        }
    }

    ExecutionResult ensureReady() {
        if (fd_ >= 0) {
            return {true, {}};
        }

        fd_ = ::open("/dev/uinput", O_WRONLY | O_NONBLOCK | O_CLOEXEC);
        if (fd_ < 0) {
            return {
                false,
                QString("Не вдалося відкрити /dev/uinput: %1").arg(QString::fromLocal8Bit(std::strerror(errno)))
            };
        }

        if (::ioctl(fd_, UI_SET_EVBIT, EV_KEY) < 0
            || ::ioctl(fd_, UI_SET_EVBIT, EV_SYN) < 0) {
            return failSetup("Не вдалося ввімкнути події клавіатури");
        }

        const int supportedKeys[] = {
            KEY_LEFTCTRL, KEY_LEFTSHIFT, KEY_LEFTALT, KEY_LEFTMETA,
            KEY_TAB, KEY_LEFT, KEY_RIGHT,
            KEY_A, KEY_C, KEY_D, KEY_V, KEY_X, KEY_Z,
            KEY_VOLUMEUP, KEY_VOLUMEDOWN, KEY_MUTE, KEY_PLAYPAUSE
        };
        for (const int code : supportedKeys) {
            if (::ioctl(fd_, UI_SET_KEYBIT, code) < 0) {
                return failSetup(QString("Не вдалося зареєструвати key code %1").arg(code));
            }
        }

        uinput_setup setup{};
        std::snprintf(setup.name, UINPUT_MAX_NAME_SIZE, "%s", "Rodavarion TDriver Virtual Keyboard");
        setup.id.bustype = BUS_USB;
        setup.id.vendor = 0x52D4;
        setup.id.product = 0x5444;
        setup.id.version = 1;

        if (::ioctl(fd_, UI_DEV_SETUP, &setup) < 0
            || ::ioctl(fd_, UI_DEV_CREATE) < 0) {
            return failSetup("Не вдалося створити віртуальну клавіатуру");
        }

        // The kernel needs a short moment to expose the newly created device
        // to the compositor before the first key sequence is emitted.
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
        return {true, {}};
    }

    ExecutionResult failSetup(const QString& prefix) {
        const auto details = QString::fromLocal8Bit(std::strerror(errno));
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
        return {false, QString("%1: %2").arg(prefix, details)};
    }

    bool emitFrame(const QList<QPair<int, int>>& events) {
        for (const auto& [code, value] : events) {
            input_event keyEvent{};
            keyEvent.type = EV_KEY;
            keyEvent.code = static_cast<__u16>(code);
            keyEvent.value = value;
            if (::write(fd_, &keyEvent, sizeof(keyEvent)) != sizeof(keyEvent)) {
                lastErrno_ = errno;
                return false;
            }
        }

        input_event syncEvent{};
        syncEvent.type = EV_SYN;
        syncEvent.code = SYN_REPORT;
        if (::write(fd_, &syncEvent, sizeof(syncEvent)) != sizeof(syncEvent)) {
            lastErrno_ = errno;
            return false;
        }
        return true;
    }

    void releaseAll(const QList<int>& modifiers, const int keyCode) {
        if (fd_ < 0) return;
        QList<QPair<int, int>> releases;
        if (keyCode >= 0) releases.push_back({keyCode, 0});
        for (auto it = modifiers.crbegin(); it != modifiers.crend(); ++it) {
            releases.push_back({*it, 0});
        }
        emitFrame(releases);
    }

    ExecutionResult lastWriteError() const {
        return {
            false,
            QString("Помилка запису у /dev/uinput: %1")
                .arg(QString::fromLocal8Bit(std::strerror(lastErrno_)))
        };
    }

    int fd_{-1};
    int lastErrno_{EIO};
    std::mutex mutex_;
};

QStringList ydotoolArguments(const QString& shortcut) {
    const auto parts = shortcutParts(shortcut);
    if (parts.isEmpty()) {
        return {};
    }

    QList<int> modifiers;
    int keyCode = -1;

    for (const auto& part : parts) {
        const int code = linuxKeyCode(part);
        if (code < 0) {
            return {};
        }

        if (part == "CTRL" || part == "CONTROL"
            || part == "SHIFT" || part == "ALT"
            || part == "META" || part == "SUPER") {
            modifiers.push_back(code);
        } else {
            keyCode = code;
        }
    }

    if (keyCode < 0) {
        return {};
    }

    QStringList result{"key", "-d", "8"};

    for (const int modifier : modifiers) {
        result << QString("%1:1").arg(modifier);
    }

    result << QString("%1:1").arg(keyCode);
    result << QString("%1:0").arg(keyCode);

    for (auto iterator = modifiers.crbegin();
         iterator != modifiers.crend();
         ++iterator) {
        result << QString("%1:0").arg(*iterator);
    }

    return result;
}

QStringList wtypeArguments(const QString& shortcut) {
    const auto parts = shortcutParts(shortcut);
    QStringList modifiers;
    QString key;

    for (const auto& part : parts) {
        if (part == "CTRL" || part == "CONTROL") {
            modifiers << "ctrl";
        } else if (part == "SHIFT") {
            modifiers << "shift";
        } else if (part == "ALT") {
            modifiers << "alt";
        } else if (part == "META" || part == "SUPER") {
            modifiers << "logo";
        } else if (part == "VOLUMEUP"
                   || part == "VOLUMEDOWN"
                   || part == "MUTE"
                   || part == "PLAYPAUSE") {
            return {};
        } else {
            key = part.toLower();
        }
    }

    if (key.isEmpty()) {
        return {};
    }

    QStringList result;

    for (const auto& modifier : modifiers) {
        result << "-M" << modifier;
    }

    result << "-k" << key;

    for (auto iterator = modifiers.crbegin();
         iterator != modifiers.crend();
         ++iterator) {
        result << "-m" << *iterator;
    }

    return result;
}

QString xdotoolKey(QString shortcut) {
    return shortcut.trimmed().toLower();
}

QString ydotoolSocketPath() {
    const auto explicitSocket =
        qEnvironmentVariable("YDOTOOL_SOCKET");

    if (!explicitSocket.isEmpty()) {
        return explicitSocket;
    }

    const auto runtimeDirectory =
        qEnvironmentVariable("XDG_RUNTIME_DIR");

    if (!runtimeDirectory.isEmpty()) {
        const auto path =
            QDir(runtimeDirectory).filePath(".ydotool_socket");

        if (QFileInfo::exists(path)) {
            return path;
        }
    }

    if (QFileInfo::exists("/tmp/.ydotool_socket")) {
        return "/tmp/.ydotool_socket";
    }

    return {};
}

bool userServiceActive() {
    QProcess process;
    process.start(
        "systemctl",
        {"--user", "is-active", "--quiet", "ydotool.service"}
    );

    if (!process.waitForFinished(1200)) {
        process.kill();
        return false;
    }

    return process.exitStatus() == QProcess::NormalExit
        && process.exitCode() == 0;
}

ExecutionResult runProcess(
    const QString& executable,
    const QStringList& arguments,
    const QProcessEnvironment& environment =
        QProcessEnvironment::systemEnvironment()
) {
    QProcess process;
    process.setProcessEnvironment(environment);
    process.start(executable, arguments);

    if (!process.waitForStarted(1200)) {
        return {
            .success = false,
            .message = QString("Не вдалося запустити %1: %2")
                .arg(executable, process.errorString())
        };
    }

    if (!process.waitForFinished(3500)) {
        process.kill();
        process.waitForFinished(500);

        return {
            .success = false,
            .message = QString("%1 не завершився вчасно.").arg(executable)
        };
    }

    const auto standardError =
        QString::fromUtf8(process.readAllStandardError()).trimmed();
    const auto standardOutput =
        QString::fromUtf8(process.readAllStandardOutput()).trimmed();

    if (process.exitStatus() != QProcess::NormalExit
        || process.exitCode() != 0) {
        const auto details = !standardError.isEmpty()
            ? standardError
            : standardOutput;

        return {
            .success = false,
            .message = QString("%1 завершився з кодом %2%3")
                .arg(executable)
                .arg(process.exitCode())
                .arg(
                    details.isEmpty()
                        ? QString(".")
                        : QString(": %1").arg(details)
                )
        };
    }

    return {
        .success = true,
        .message = QString("Комбінацію передано через %1.").arg(executable)
    };
}

ExecutionResult executeWithYdotool(const QString& shortcut) {
    const auto executable =
        QStandardPaths::findExecutable("ydotool");

    if (executable.isEmpty()) {
        return {
            .success = false,
            .message = "ydotool не встановлено."
        };
    }

    const auto arguments = ydotoolArguments(shortcut);
    if (arguments.isEmpty()) {
        return {
            .success = false,
            .message = "Не вдалося перетворити комбінацію на Linux key codes."
        };
    }

    const auto socket = ydotoolSocketPath();
    if (socket.isEmpty() && !userServiceActive()) {
        return {
            .success = false,
            .message =
                "Служба ydotool не активна. Виконайте "
                "tools/setup_ydotool_arch.sh і перезайдіть у сеанс."
        };
    }

    auto environment = QProcessEnvironment::systemEnvironment();
    if (!socket.isEmpty()) {
        environment.insert("YDOTOOL_SOCKET", socket);
    }

    return runProcess(executable, arguments, environment);
}

} // namespace

ExecutionResult DesktopActionExecutor::execute(
    const action::ActionType action,
    const QString& parameter
) {
    if (action == action::ActionType::Default
        || action == action::ActionType::Disabled) {
        return {
            .success = true,
            .message = "Дію не призначено."
        };
    }

    if (action == action::ActionType::RunScript) {
        const QFileInfo script(parameter);
        if (!script.isAbsolute() || !script.exists() || !script.isFile() || script.isSymLink()) {
            return {false, "Скрипт не пройшов перевірку безпеки."};
        }
        QProcess process;
        process.setProgram(script.absoluteFilePath());
        process.setWorkingDirectory(script.absolutePath());
        process.setProcessChannelMode(QProcess::MergedChannels);
        process.start();
        if (!process.waitForStarted(1500)) return {false, "Не вдалося запустити скрипт: " + process.errorString()};
        if (!process.waitForFinished(10000)) { process.kill(); process.waitForFinished(500); return {false, "Скрипт зупинено: перевищено ліміт 10 секунд."}; }
        const QString output = QString::fromUtf8(process.readAll()).trimmed();
        if (process.exitStatus()!=QProcess::NormalExit || process.exitCode()!=0)
            return {false, QString("Скрипт завершився з кодом %1%2").arg(process.exitCode()).arg(output.isEmpty()?QString():QString(": %1").arg(output))};
        return {true, output.isEmpty()?QString("Скрипт виконано успішно."):QString("Скрипт виконано: %1").arg(output.left(500))};
    }

    if (action == action::ActionType::CustomShortcut) {
        const auto custom = parameter.trimmed();
        if (custom.isEmpty()) {
            return {
                .success = false,
                .message = "Власна комбінація порожня."
            };
        }

        // Compatibility with mappings created before semantic Copy/Paste
        // actions existed. Exact Ctrl+C/Ctrl+V mappings become context-aware,
        // while all other custom shortcuts remain literal.
        const auto normalized = custom.toUpper().remove(' ');
        if (normalized == "CTRL+C") {
            const auto contextual = context::ApplicationContext::shortcutFor(action::ActionType::Copy);
            return contextual.isEmpty()
                ? ExecutionResult{false, "Активну програму не визначено. Комбінацію не надіслано."}
                : executeShortcut(contextual);
        }
        if (normalized == "CTRL+V") {
            const auto contextual = context::ApplicationContext::shortcutFor(action::ActionType::Paste);
            return contextual.isEmpty()
                ? ExecutionResult{false, "Активну програму не визначено. Комбінацію не надіслано."}
                : executeShortcut(contextual);
        }

        return executeShortcut(custom);
    }

    auto shortcut = context::ApplicationContext::shortcutFor(action);
    const bool contextSensitive = action == action::ActionType::Copy
        || action == action::ActionType::Paste
        || action == action::ActionType::Cut
        || action == action::ActionType::Undo
        || action == action::ActionType::Redo
        || action == action::ActionType::SelectAll;
    if (shortcut.isEmpty() && contextSensitive) {
        return {
            .success = false,
            .message = "Активну програму не визначено. Комбінацію не надіслано, щоб уникнути помилкової дії."
        };
    }
    if (shortcut.isEmpty()) {
        shortcut = shortcutForAction(action);
    }
    if (shortcut.isEmpty()) {
        return {
            .success = false,
            .message = "Для цієї дії ще немає виконавчого бекенду."
        };
    }

    return executeShortcut(shortcut);
}

QString DesktopActionExecutor::backendDescription() {
    const auto sessionType =
        qEnvironmentVariable("XDG_SESSION_TYPE").toLower();

    if (DirectKeyboardDevice::instance().available()) {
        return "Linux /dev/uinput — готовий (вбудований бекенд)";
    }

    if (sessionType == "wayland") {
        if (!QStandardPaths::findExecutable("ydotool").isEmpty()) {
            return userServiceActive() || !ydotoolSocketPath().isEmpty()
                ? "Wayland / ydotool — готовий"
                : "Wayland / ydotool — служба не активна";
        }

        if (!QStandardPaths::findExecutable("wtype").isEmpty()) {
            return "Wayland / wtype — сумісність залежить від compositor";
        }

        return "Wayland — виконавчий бекенд відсутній";
    }

    if (!QStandardPaths::findExecutable("xdotool").isEmpty()) {
        return "X11 / xdotool — готовий";
    }

    if (!QStandardPaths::findExecutable("ydotool").isEmpty()) {
        return userServiceActive() || !ydotoolSocketPath().isEmpty()
            ? "ydotool — готовий"
            : "ydotool — служба не активна";
    }

    return "Виконавчий бекенд відсутній";
}

QString DesktopActionExecutor::diagnosticReport() {
    return QString(
        "Сеанс: %1\n"
        "KDE desktop: %2\n"
        "Вбудований /dev/uinput: %3\n"
        "ydotool: %4\n"
        "ydotool.service: %5\n"
        "ydotool socket: %6\n"
        "wtype: %7\n"
        "xdotool: %8\n"
        "Обраний бекенд: %9"
    )
        .arg(qEnvironmentVariable("XDG_SESSION_TYPE", "unknown"))
        .arg(qEnvironmentVariable("XDG_CURRENT_DESKTOP", "unknown"))
        .arg(DirectKeyboardDevice::instance().status())
        .arg(QStandardPaths::findExecutable("ydotool").isEmpty()
            ? "не встановлено" : QStandardPaths::findExecutable("ydotool"))
        .arg(userServiceActive() ? "active" : "inactive")
        .arg(ydotoolSocketPath().isEmpty() ? "не знайдено" : ydotoolSocketPath())
        .arg(QStandardPaths::findExecutable("wtype").isEmpty()
            ? "не встановлено" : QStandardPaths::findExecutable("wtype"))
        .arg(QStandardPaths::findExecutable("xdotool").isEmpty()
            ? "не встановлено" : QStandardPaths::findExecutable("xdotool"))
        .arg(backendDescription());
}

ExecutionResult DesktopActionExecutor::executeShortcut(
    const QString& portableShortcut
) {
    // Primary backend: emit a real virtual keyboard through the kernel. This
    // works independently of X11/Wayland, graphical-session environment
    // variables and ydotool socket state.
    const auto directResult =
        DirectKeyboardDevice::instance().sendShortcut(portableShortcut);
    if (directResult.success) {
        return directResult;
    }

    const auto sessionType =
        qEnvironmentVariable("XDG_SESSION_TYPE").toLower();

    if (sessionType == "wayland") {
        const auto ydotoolResult = executeWithYdotool(portableShortcut);
        if (ydotoolResult.success) {
            return ydotoolResult;
        }

        const auto wtype = QStandardPaths::findExecutable("wtype");
        if (!wtype.isEmpty()) {
            const auto arguments = wtypeArguments(portableShortcut);
            if (!arguments.isEmpty()) {
                const auto result = runProcess(wtype, arguments);
                if (result.success) return result;
            }
        }

        return {
            false,
            QString("Вбудований бекенд: %1; резервний ydotool: %2")
                .arg(directResult.message, ydotoolResult.message)
        };
    }

    const auto xdotool = QStandardPaths::findExecutable("xdotool");
    if (!xdotool.isEmpty()) {
        const auto result = runProcess(
            xdotool,
            {"key", "--clearmodifiers", xdotoolKey(portableShortcut)}
        );
        if (result.success) return result;
    }

    const auto ydotoolResult = executeWithYdotool(portableShortcut);
    if (ydotoolResult.success) return ydotoolResult;

    return {
        false,
        QString("Вбудований бекенд: %1; резервний ydotool: %2")
            .arg(directResult.message, ydotoolResult.message)
    };
}

} // namespace rodavarion::runtime
