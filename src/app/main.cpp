#include "rodavarion/core/Application.hpp"
#include "rodavarion/core/Logger.hpp"
#include "rodavarion/gui/MainWindow.hpp"
#include "rodavarion/i18n/LocalizationService.hpp"

#include <QApplication>
#include <QMessageBox>
#include <QLockFile>
#include <QStandardPaths>
#include <QDir>
#include <QSettings>
#include <QTimer>
#include <QSystemTrayIcon>

#include <exception>

int main(int argc, char* argv[]) {
    QApplication qtApplication(argc, argv);
    QApplication::setApplicationName("Rodavarion TDriver");
    QApplication::setApplicationVersion(QStringLiteral(RODAVARION_VERSION_STRING));
    QApplication::setOrganizationName("Rodavarion Technologies");
    QApplication::setQuitOnLastWindowClosed(false);

    const auto runtimeDir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    QDir().mkpath(runtimeDir);
    QLockFile instanceLock(runtimeDir + "/rodavarion-tdriver.lock");
    instanceLock.setStaleLockTime(30000);
    if (!instanceLock.tryLock(100)) {
        QMessageBox::information(
            nullptr,
            "Rodavarion TDriver",
            "Rodavarion TDriver уже запущено. Скористайтеся значком програми у системному треї."
        );
        return 0;
    }

    rodavarion::i18n::LocalizationService::instance().load();

    try {
        rodavarion::core::Application application;
        application.initialize();

        rodavarion::gui::MainWindow window(application);
        const QStringList arguments = qtApplication.arguments();
        QSettings settings;
        const bool requestedTray = arguments.contains("--tray");
        const bool startInTray = requestedTray
            || settings.value("startup/startInTray", true).toBool();
        if (startInTray && QSystemTrayIcon::isSystemTrayAvailable()) {
            QTimer::singleShot(0, &window, [&window]() { window.hide(); });
        } else {
            window.show();
        }

        return qtApplication.exec();
    } catch (const std::exception& error) {
        rodavarion::core::Logger::instance().log(
            rodavarion::core::LogLevel::Error,
            error.what()
        );

        QMessageBox::critical(
            nullptr,
            "Rodavarion TDriver",
            QString("Fatal error:\n%1").arg(error.what())
        );
        return 1;
    } catch (...) {
        QMessageBox::critical(
            nullptr,
            "Rodavarion TDriver",
            "Unknown fatal error."
        );
        return 2;
    }
}
