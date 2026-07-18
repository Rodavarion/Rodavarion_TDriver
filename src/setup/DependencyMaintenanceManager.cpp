#include "rodavarion/setup/DependencyMaintenanceManager.hpp"

#include "rodavarion/capability/DeviceCapability.hpp"
#include "rodavarion/transport/TransportType.hpp"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QSaveFile>
#include <QSet>
#include <QStandardPaths>

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
    const int timeoutMs = 180000
) {
    QProcess process;
    auto environment = QProcessEnvironment::systemEnvironment();
    environment.insert("LC_ALL", "C");
    process.setProcessEnvironment(environment);
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(program, arguments);

    if (!process.waitForStarted(3000)) {
        return {
            false,
            -1,
            QString("Не вдалося запустити %1: %2")
                .arg(program, process.errorString())
        };
    }

    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        process.waitForFinished(1000);
        return {
            false,
            -1,
            QString("%1 перевищив час очікування.").arg(program)
        };
    }

    return {
        process.exitStatus() == QProcess::NormalExit
            && process.exitCode() == 0,
        process.exitCode(),
        QString::fromUtf8(process.readAll()).trimmed()
    };
}

QJsonObject readObject(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(
        file.readAll(),
        &error
    );

    return error.error == QJsonParseError::NoError
            && document.isObject()
        ? document.object()
        : QJsonObject{};
}

bool writeObject(
    const QString& path,
    const QJsonObject& object
) {
    const QFileInfo info(path);
    QDir().mkpath(info.absolutePath());

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(
        QJsonDocument(object).toJson(
            QJsonDocument::Indented
        )
    );

    return file.commit();
}

QStringList jsonStrings(const QJsonValue& value) {
    QStringList result;

    for (const auto& item : value.toArray()) {
        if (item.isString()) {
            result.push_back(item.toString());
        }
    }

    result.removeDuplicates();
    result.sort();
    return result;
}

QJsonArray toArray(const QStringList& values) {
    QJsonArray result;
    for (const auto& value : values) {
        result.append(value);
    }
    return result;
}

bool packageInstalled(const QString& package) {
    return run(
        "/usr/bin/pacman",
        {"-Q", "--", package},
        10000
    ).success;
}

bool packageHasReverseDependencies(
    const QString& package
) {
    const auto result = run(
        "/usr/bin/pacman",
        {"-Qi", "--", package},
        10000
    );

    if (!result.success) {
        return true;
    }

    const auto lines = result.output.split('\n');

    for (const auto& line : lines) {
        if (!line.startsWith("Required By")) {
            continue;
        }

        const auto value =
            line.section(':', 1).trimmed();

        return !value.isEmpty() && value != "None";
    }

    // Unknown package metadata is treated as unsafe to remove.
    return true;
}

QStringList allPackagesForProfiles(
    const QJsonObject& manifest,
    const QStringList& profiles
) {
    QStringList packages;
    const auto profileObject =
        manifest.value("profiles").toObject();

    for (const auto& profile : profiles) {
        const auto entry =
            profileObject.value(profile).toObject();

        packages.append(
            jsonStrings(entry.value("packages"))
        );
    }

    packages.removeDuplicates();
    packages.sort();
    return packages;
}

QStringList protectedPackages(
    const QJsonObject& manifest
) {
    QStringList values = jsonStrings(
        manifest.value("protected_packages")
    );

    values.append({
        "base",
        "base-devel",
        "glibc",
        "gcc",
        "linux",
        "linux-lts",
        "pacman",
        "systemd",
        "qt6-base",
        "cmake",
        "ninja",
        "polkit",
        "hidapi",
        "libevdev"
    });

    values.removeDuplicates();
    values.sort();
    return values;
}

void addProfile(
    QStringList& profiles,
    const QString& profile
) {
    if (!profiles.contains(profile)) {
        profiles.push_back(profile);
    }
}

} // namespace

QString DependencyMaintenanceManager::ledgerPath() {
    return QDir::home().filePath(
        ".local/share/rodavarion-tdriver/"
        "package-ledger.arch.json"
    );
}

QString DependencyMaintenanceManager::featureStatePath() {
    return QDir::home().filePath(
        ".config/rodavarion-tdriver/"
        "enabled-features.json"
    );
}

QString DependencyMaintenanceManager::manifestPath() {
    const QStringList candidates{
        QDir::current().filePath(
            "config/dependencies.arch.json"
        ),
        QDir(QCoreApplication::applicationDirPath())
            .filePath("../config/dependencies.arch.json"),
        QDir(QCoreApplication::applicationDirPath())
            .filePath(
                "../share/rodavarion-tdriver/config/"
                "dependencies.arch.json"
            ),
        "/usr/share/rodavarion-tdriver/config/"
        "dependencies.arch.json"
    };

    for (const auto& candidate : candidates) {
        if (QFile::exists(candidate)) {
            return QFileInfo(candidate)
                .canonicalFilePath();
        }
    }

    return {};
}

void DependencyMaintenanceManager::recordDeviceRequirements(
    const std::vector<device::PeripheralDescriptor>& peripherals
) {
    auto state = readObject(featureStatePath());
    QStringList profiles = jsonStrings(
        state.value("enabled_profiles")
    );

    addProfile(profiles, "runtime-core");

    for (const auto& peripheral : peripherals) {
        using capability::DeviceClass;

        switch (peripheral.classification.deviceClass) {
            case DeviceClass::Printer:
            case DeviceClass::ThermalPrinter:
            case DeviceClass::LabelPrinter:
            case DeviceClass::ReceiptPrinter:
                addProfile(profiles, "printing");
                break;

            case DeviceClass::DocumentScanner:
            case DeviceClass::BarcodeScanner:
                addProfile(profiles, "scanning");
                break;

            default:
                break;
        }

        if (peripheral.transport
            == transport::TransportType::Bluetooth) {
            addProfile(profiles, "bluetooth");
        }

        if (peripheral.transport
            == transport::TransportType::Serial) {
            addProfile(profiles, "serial");
        }
    }

    profiles.removeDuplicates();
    profiles.sort();

    state.insert("schema_version", 1);
    state.insert(
        "enabled_profiles",
        toArray(profiles)
    );
    state.insert(
        "updated_at",
        QDateTime::currentDateTimeUtc()
            .toString(Qt::ISODate)
    );

    writeObject(featureStatePath(), state);
}

MaintenanceAudit DependencyMaintenanceManager::audit() {
    MaintenanceAudit audit;

    const auto manifest = readObject(manifestPath());
    const auto state = readObject(featureStatePath());
    const auto ledger = readObject(ledgerPath());

    audit.enabledProfiles = jsonStrings(
        state.value("enabled_profiles")
    );

    if (audit.enabledProfiles.isEmpty()) {
        audit.enabledProfiles = {"runtime-core"};
    }

    audit.requiredPackages = allPackagesForProfiles(
        manifest,
        audit.enabledProfiles
    );

    audit.installedByRodavarion = jsonStrings(
        ledger.value("installed_by_rodavarion")
    );

    audit.protectedPackages =
        protectedPackages(manifest);

    for (const auto& package : audit.requiredPackages) {
        if (!packageInstalled(package)) {
            audit.missingRequiredPackages.push_back(
                package
            );
        }
    }

    const QSet<QString> required(
        audit.requiredPackages.begin(),
        audit.requiredPackages.end()
    );
    const QSet<QString> protectedSet(
        audit.protectedPackages.begin(),
        audit.protectedPackages.end()
    );

    for (const auto& package
         : audit.installedByRodavarion) {
        if (!packageInstalled(package)
            || required.contains(package)
            || protectedSet.contains(package)
            || packageHasReverseDependencies(package)) {
            continue;
        }

        audit.removablePackages.push_back(package);
    }

    audit.missingRequiredPackages.removeDuplicates();
    audit.missingRequiredPackages.sort();
    audit.removablePackages.removeDuplicates();
    audit.removablePackages.sort();

    audit.summary = QString(
        "Активні профілі: %1\n"
        "Пакетів, встановлених Rodavarion TDriver: %2\n"
        "Потрібних пакетів: %3\n"
        "Відсутніх потрібних пакетів: %4\n"
        "Безпечно доступних для видалення: %5\n\n"
        "Програма ніколи не пропонує видаляти пакет, "
        "який був установлений до Rodavarion TDriver, "
        "потрібний активному профілю, входить до "
        "захищеного списку або має залежні пакети."
    )
        .arg(audit.enabledProfiles.join(", "))
        .arg(audit.installedByRodavarion.size())
        .arg(audit.requiredPackages.size())
        .arg(audit.missingRequiredPackages.size())
        .arg(audit.removablePackages.size());

    return audit;
}

MaintenanceResult
DependencyMaintenanceManager::installMissingRequiredPackages() {
    const auto current = audit();

    if (current.missingRequiredPackages.isEmpty()) {
        return {
            true,
            "Усі залежності активних профілів уже встановлено."
        };
    }

    QStringList arguments{
        "/usr/bin/pacman",
        "-S",
        "--needed",
        "--noconfirm",
        "--"
    };
    arguments.append(current.missingRequiredPackages);

    const auto result = run(
        "pkexec",
        arguments
    );

    if (!result.success) {
        return {
            false,
            result.output.isEmpty()
                ? QString("pacman завершився з кодом %1.")
                    .arg(result.exitCode)
                : result.output
        };
    }

    auto ledger = readObject(ledgerPath());
    auto installed = jsonStrings(
        ledger.value("installed_by_rodavarion")
    );
    installed.append(current.missingRequiredPackages);
    installed.removeDuplicates();
    installed.sort();

    ledger.insert("schema_version", 1);
    ledger.insert(
        "installed_by_rodavarion",
        toArray(installed)
    );
    ledger.insert(
        "updated_at",
        QDateTime::currentDateTimeUtc()
            .toString(Qt::ISODate)
    );
    writeObject(ledgerPath(), ledger);

    return {
        true,
        QString("Встановлено відсутні пакети: %1")
            .arg(
                current.missingRequiredPackages.join(", ")
            )
    };
}

MaintenanceResult
DependencyMaintenanceManager::removeUnusedRodavarionPackages() {
    const auto current = audit();

    if (current.removablePackages.isEmpty()) {
        return {
            true,
            "Безпечного баласту для видалення не знайдено."
        };
    }

    QStringList arguments{
        "/usr/bin/pacman",
        "-R",
        "--noconfirm",
        "--"
    };
    arguments.append(current.removablePackages);

    const auto result = run(
        "pkexec",
        arguments
    );

    if (!result.success) {
        return {
            false,
            result.output.isEmpty()
                ? QString("pacman завершився з кодом %1.")
                    .arg(result.exitCode)
                : result.output
        };
    }

    auto ledger = readObject(ledgerPath());
    auto installed = jsonStrings(
        ledger.value("installed_by_rodavarion")
    );

    for (const auto& package
         : current.removablePackages) {
        installed.removeAll(package);
    }

    ledger.insert(
        "installed_by_rodavarion",
        toArray(installed)
    );
    ledger.insert(
        "updated_at",
        QDateTime::currentDateTimeUtc()
            .toString(Qt::ISODate)
    );
    writeObject(ledgerPath(), ledger);

    return {
        true,
        QString("Видалено лише пакети з журналу "
                "Rodavarion TDriver: %1")
            .arg(current.removablePackages.join(", "))
    };
}

} // namespace rodavarion::setup
