#include "rodavarion/setup/DependencySetupManager.hpp"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QStringList>

namespace rodavarion::setup {

namespace {

struct CommandResult {
    bool success{false};
    int exitCode{-1};
    QString output;
};

CommandResult run(
    const QString& program,
    const QStringList& arguments,
    const int timeoutMs = 120000
) {
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(program, arguments);

    if (!process.waitForStarted(3000)) {
        return {
            .success = false,
            .exitCode = -1,
            .output = QString("Не вдалося запустити %1: %2")
                .arg(program, process.errorString())
        };
    }

    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        process.waitForFinished(1000);

        return {
            .success = false,
            .exitCode = -1,
            .output = QString("Команда %1 перевищила час очікування.")
                .arg(program)
        };
    }

    const auto output =
        QString::fromUtf8(process.readAll()).trimmed();

    return {
        .success =
            process.exitStatus() == QProcess::NormalExit
            && process.exitCode() == 0,
        .exitCode = process.exitCode(),
        .output = output
    };
}

QString yesNo(const bool value) {
    return value ? "так" : "ні";
}

} // namespace

bool DependencySetupManager::commandExists(
    const QString& command
) {
    return !QStandardPaths::findExecutable(command).isEmpty();
}

bool DependencySetupManager::userServiceActive(
    const QString& service
) {
    const auto result = run(
        "systemctl",
        {"--user", "is-active", "--quiet", service},
        3000
    );
    return result.success;
}

ActionBackendStatus DependencySetupManager::inspect() {
    ActionBackendStatus status;

    status.archLinux =
        QFile::exists("/etc/arch-release");
    status.pacmanAvailable =
        commandExists("pacman");
    status.pkexecAvailable =
        commandExists("pkexec");
    status.ydotoolInstalled =
        commandExists("ydotool")
        && commandExists("ydotoold");
    status.uinputPresent =
        QFile::exists("/dev/uinput");
    status.ydotoolServiceActive =
        userServiceActive("ydotool.service");

    const auto session =
        qEnvironmentVariable("XDG_SESSION_TYPE").toLower();

    if (session == "wayland") {
        status.ready =
            status.ydotoolInstalled
            && status.uinputPresent
            && status.ydotoolServiceActive;
    } else {
        status.ready =
            commandExists("xdotool")
            || (
                status.ydotoolInstalled
                && status.uinputPresent
                && status.ydotoolServiceActive
            );
    }

    status.summary = QString(
        "Arch Linux: %1\n"
        "pacman: %2\n"
        "pkexec / Polkit: %3\n"
        "ydotool і ydotoold: %4\n"
        "/dev/uinput: %5\n"
        "ydotool.service: %6\n"
        "Стан: %7"
    )
        .arg(yesNo(status.archLinux))
        .arg(yesNo(status.pacmanAvailable))
        .arg(yesNo(status.pkexecAvailable))
        .arg(yesNo(status.ydotoolInstalled))
        .arg(yesNo(status.uinputPresent))
        .arg(yesNo(status.ydotoolServiceActive))
        .arg(
            status.ready
                ? "готово до виконання дій"
                : "потрібне налаштування"
        );

    return status;
}

SetupResult DependencySetupManager::installAndConfigure() {
    const auto before = inspect();

    if (!before.archLinux || !before.pacmanAvailable) {
        return {
            .success = false,
            .details =
                "Автоматичний інсталятор цієї версії підтримує "
                "Arch Linux і сумісні системи з pacman."
        };
    }

    if (!before.pkexecAvailable) {
        return {
            .success = false,
            .details =
                "Не знайдено pkexec. Програма не виконуватиме "
                "адміністративні зміни без стандартного Polkit-запиту."
        };
    }

    QStringList log;

    if (!before.ydotoolInstalled) {
        const auto install = run(
            "pkexec",
            {
                "/usr/bin/pacman",
                "-S",
                "--needed",
                "--noconfirm",
                "ydotool"
            }
        );

        log << "Встановлення ydotool:"
            << (
                install.output.isEmpty()
                    ? QString("код %1").arg(install.exitCode)
                    : install.output
            );

        if (!install.success) {
            return {
                .success = false,
                .details = log.join("\n")
            };
        }
    } else {
        log << "ydotool уже встановлено.";
    }

    // Load the kernel module for the current session.
    const auto modprobe = run(
        "pkexec",
        {"/usr/bin/modprobe", "uinput"},
        15000
    );

    log << "Завантаження uinput:"
        << (
            modprobe.success
                ? "успішно"
                : (
                    modprobe.output.isEmpty()
                        ? QString("код %1").arg(modprobe.exitCode)
                        : modprobe.output
                )
        );

    if (!modprobe.success) {
        return {
            .success = false,
            .details = log.join("\n")
        };
    }

    // Persist the fixed module name without executing a shell command.
    QTemporaryFile moduleFile(
        QDir::tempPath()
        + "/rodavarion-uinput-XXXXXX.conf"
    );

    if (!moduleFile.open()) {
        log << "Не вдалося створити тимчасовий файл modules-load.";
        return {
            .success = false,
            .details = log.join("\n")
        };
    }

    moduleFile.write("uinput\n");
    moduleFile.flush();

    const auto installModuleConfig = run(
        "pkexec",
        {
            "/usr/bin/install",
            "-m",
            "0644",
            moduleFile.fileName(),
            "/etc/modules-load.d/rodavarion-uinput.conf"
        },
        15000
    );

    log << "Автозавантаження uinput:"
        << (
            installModuleConfig.success
                ? "налаштовано"
                : (
                    installModuleConfig.output.isEmpty()
                        ? QString("код %1")
                              .arg(installModuleConfig.exitCode)
                        : installModuleConfig.output
                )
        );

    if (!installModuleConfig.success) {
        return {
            .success = false,
            .details = log.join("\n")
        };
    }

    // Reload packaged udev rules and trigger only uinput.
    const auto reloadRules = run(
        "pkexec",
        {"/usr/bin/udevadm", "control", "--reload-rules"},
        15000
    );
    log << QString("Перезавантаження udev-правил: %1")
        .arg(reloadRules.success ? "успішно" : "неуспішно");

    const auto trigger = run(
        "pkexec",
        {
            "/usr/bin/udevadm",
            "trigger",
            "--name-match=uinput"
        },
        15000
    );
    log << QString("Активація uinput: %1")
        .arg(trigger.success ? "успішно" : "неуспішно");

    // Start the official packaged user service in the current session.
    const auto daemonReload = run(
        "systemctl",
        {"--user", "daemon-reload"},
        10000
    );
    log << QString("systemd user daemon-reload: %1")
        .arg(daemonReload.success ? "успішно" : "неуспішно");

    const auto enableService = run(
        "systemctl",
        {
            "--user",
            "enable",
            "--now",
            "ydotool.service"
        },
        20000
    );

    log << "Запуск ydotool.service:"
        << (
            enableService.success
                ? "успішно"
                : (
                    enableService.output.isEmpty()
                        ? QString("код %1")
                              .arg(enableService.exitCode)
                        : enableService.output
                )
        );

    const auto after = inspect();
    log << "\nПідсумковий стан:"
        << after.summary;

    return {
        .success = after.ready,
        .details = log.join("\n")
    };
}

} // namespace rodavarion::setup
