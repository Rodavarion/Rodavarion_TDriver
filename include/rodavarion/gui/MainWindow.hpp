#pragma once

#include "rodavarion/core/Application.hpp"
#include "rodavarion/device/PeripheralDescriptor.hpp"

#include <QMainWindow>

#include <memory>
#include <vector>

class QActionGroup;
class QCloseEvent;
class QMenu;
class QComboBox;
class QCheckBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QProgressBar;
class QStackedWidget;
class QTableWidget;
class QSystemTrayIcon;
class QTimer;
class QString;

namespace rodavarion::runtime {
class MouseRuntimeController;
}

namespace rodavarion::gui {

class MainWindow final : public QMainWindow {
public:
    explicit MainWindow(core::Application& application);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void buildInterface();
    void applyTheme();
    void buildLanguageMenu();
    void applyLanguage();
    void restoreWindowState();
    void saveGeneralSettings();
    void updateAutostartEntry(bool enabled);
    void createTrayIcon();
    void recoverTrayIcon();
    void checkDisplayScaling();
    void showFromTray();
    void quitApplication();
    void showWelcomeDialogIfNeeded();
    void openExternalUrl(const QString& url);
    void copyDonationCard();
    void refreshDevices();
    void showSelectedDeviceDetails();
    void openSelectedPeripheral();
    void loadSelectedProfile();
    void saveSelectedProfile();
    void restartMouseRuntime();
    void ensureActionBackend(bool userInitiated = false);
    void refreshDependencyMaintenance();
    void installMissingDependencies();
    void removeUnusedDependencies();
    void setMaintenanceBusy(bool busy, const QString& actionText = {});
    void setMaintenanceFeedback(const QString& message, bool success, bool warning = false);

    core::Application& application_;
    std::vector<device::PeripheralDescriptor> peripherals_;

    QListWidget* navigation_{nullptr};
    QStackedWidget* pages_{nullptr};

    QTableWidget* deviceTable_{nullptr};
    QTableWidget* interfaceTable_{nullptr};
    QLabel* deviceDetails_{nullptr};
    QLabel* statusLabel_{nullptr};
    QPushButton* refreshButton_{nullptr};

    QListWidget* profileList_{nullptr};
    QLabel* profileNameLabel_{nullptr};
    QLabel* pointerSpeedLabel_{nullptr};
    QLabel* scrollModeLabel_{nullptr};
    QLineEdit* profileNameEdit_{nullptr};
    QLineEdit* pointerSpeedEdit_{nullptr};
    QComboBox* scrollModeCombo_{nullptr};
    QPushButton* saveProfileButton_{nullptr};

    QTableWidget* applicationProfilesTable_{nullptr};
    QLabel* applicationContextStatusLabel_{nullptr};
    QPushButton* saveApplicationProfilesButton_{nullptr};
    QPushButton* resetApplicationProfilesButton_{nullptr};

    QCheckBox* autostartCheckBox_{nullptr};
    QCheckBox* startInTrayCheckBox_{nullptr};
    QCheckBox* rememberGeometryCheckBox_{nullptr};
    QCheckBox* rememberLastPageCheckBox_{nullptr};
    QCheckBox* restoreProfileCheckBox_{nullptr};
    QPushButton* saveGeneralSettingsButton_{nullptr};
    QLabel* generalSettingsStatusLabel_{nullptr};

    QLabel* diagnosticsLabel_{nullptr};
    QLabel* aboutTitleLabel_{nullptr};
    QLabel* aboutCompanyLabel_{nullptr};
    QLabel* aboutMottoLabel_{nullptr};
    QLabel* aboutDescriptionLabel_{nullptr};
    QLabel* aboutVersionLabel_{nullptr};
    QLabel* supportStatusLabel_{nullptr};
    QLabel* maintenanceTitleLabel_{nullptr};
    QLabel* maintenanceDescriptionLabel_{nullptr};
    QLabel* maintenanceStatusLabel_{nullptr};
    QListWidget* maintenanceMissingList_{nullptr};
    QListWidget* maintenanceRemovableList_{nullptr};
    QPushButton* maintenanceRefreshButton_{nullptr};
    QPushButton* maintenanceInstallButton_{nullptr};
    QPushButton* maintenanceCleanupButton_{nullptr};
    QLabel* maintenanceOperationLabel_{nullptr};
    QLabel* maintenanceLastRunLabel_{nullptr};
    QProgressBar* maintenanceProgressBar_{nullptr};
    bool maintenanceBusy_{false};
    QMenu* settingsMenu_{nullptr};
    QMenu* languageMenu_{nullptr};
    QActionGroup* languageGroup_{nullptr};
    std::unique_ptr<runtime::MouseRuntimeController> mouseRuntime_;
    QSystemTrayIcon* trayIcon_{nullptr};
    QMenu* trayMenu_{nullptr};
    bool quitting_{false};
    bool trayHintShown_{false};
    QTimer* trayRecoveryTimer_{nullptr};
    QTimer* displayScaleTimer_{nullptr};
    int lastScreenCount_{0};
};

} // namespace rodavarion::gui
