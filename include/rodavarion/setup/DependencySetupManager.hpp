#pragma once

#include <QString>

namespace rodavarion::setup {

struct ActionBackendStatus {
    bool archLinux{false};
    bool pacmanAvailable{false};
    bool pkexecAvailable{false};
    bool ydotoolInstalled{false};
    bool uinputPresent{false};
    bool ydotoolServiceActive{false};
    bool ready{false};

    QString summary;
};

struct SetupResult {
    bool success{false};
    QString details;
};

class DependencySetupManager final {
public:
    [[nodiscard]] static ActionBackendStatus inspect();

    // Performs only fixed, allow-listed system changes:
    // 1. install the official Arch ydotool package through pacman;
    // 2. load uinput for the current boot;
    // 3. install a fixed modules-load configuration;
    // 4. enable/start the packaged per-user ydotool service.
    [[nodiscard]] static SetupResult installAndConfigure();

private:
    [[nodiscard]] static bool commandExists(const QString& command);
    [[nodiscard]] static bool userServiceActive(
        const QString& service
    );
};

} // namespace rodavarion::setup
