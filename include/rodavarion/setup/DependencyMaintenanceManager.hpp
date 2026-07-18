#pragma once

#include "rodavarion/device/PeripheralDescriptor.hpp"

#include <QString>
#include <QStringList>

#include <vector>

namespace rodavarion::setup {

struct MaintenanceAudit {
    QStringList enabledProfiles;
    QStringList installedByRodavarion;
    QStringList requiredPackages;
    QStringList missingRequiredPackages;
    QStringList removablePackages;
    QStringList protectedPackages;
    QString summary;
};

struct MaintenanceResult {
    bool success{false};
    QString details;
};

class DependencyMaintenanceManager final {
public:
    [[nodiscard]] static MaintenanceAudit audit();

    static void recordDeviceRequirements(
        const std::vector<device::PeripheralDescriptor>& peripherals
    );

    [[nodiscard]] static MaintenanceResult
        installMissingRequiredPackages();

    [[nodiscard]] static MaintenanceResult
        removeUnusedRodavarionPackages();

    [[nodiscard]] static QString ledgerPath();
    [[nodiscard]] static QString featureStatePath();

private:
    [[nodiscard]] static QString manifestPath();
};

} // namespace rodavarion::setup
