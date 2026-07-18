#pragma once

#include "rodavarion/action/MouseAction.hpp"
#include "rodavarion/device/PeripheralDescriptor.hpp"
#include "rodavarion/input/EvdevMouseMonitor.hpp"

#include <QDialog>
#include <QVector>

#include <memory>
#include <set>
#include <vector>

class QComboBox;
class QDialogButtonBox;
class QLabel;
class QListWidget;
class QPushButton;
class QResizeEvent;
class QTableWidget;

namespace rodavarion::gui {

class MouseActionDialog final : public QDialog {
public:
    explicit MouseActionDialog(
        const device::PeripheralDescriptor& peripheral,
        QWidget* parent = nullptr
    );
    ~MouseActionDialog() override;

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void buildInterface();
    void loadMappings();
    void saveMappings();
    void updateParameterEditor(int row);
    void selectMappingRow(int row);
    void runDryTest(int row);
    void appendTestMessage(const QString& message);

    void refreshInputDevices();
    void toggleLiveMonitor();
    void startSelectedMonitors();
    void stopAllMonitors() noexcept;
    void connectMonitor(input::EvdevMouseMonitor& monitor);

    void handleLiveButton(
        action::MouseButton button,
        bool pressed,
        int linuxCode
    );
    void markButtonConfirmed(action::MouseButton button);
    void flashMappingRow(int row);
    void applyResponsiveSizing();

    [[nodiscard]] std::set<action::MouseButton>
        inferredButtons() const;
    [[nodiscard]] bool deviceMatchesPeripheral(
        const input::EvdevDeviceInfo& device
    ) const;

    device::PeripheralDescriptor peripheral_;
    action::MouseMappingProfile profile_;
    std::set<action::MouseButton> confirmedButtons_;

    QVector<input::EvdevDeviceInfo> discoveredInputDevices_;
    std::vector<std::unique_ptr<input::EvdevMouseMonitor>> monitors_;

    QLabel* deviceLabel_{nullptr};
    QLabel* noteLabel_{nullptr};

    QTableWidget* mappingTable_{nullptr};
    QListWidget* testLog_{nullptr};
    QPushButton* clearTestButton_{nullptr};

    QComboBox* inputDeviceCombo_{nullptr};
    QPushButton* refreshInputButton_{nullptr};
    QPushButton* liveMonitorButton_{nullptr};
    QLabel* liveMonitorStatus_{nullptr};

    QDialogButtonBox* buttons_{nullptr};
};

} // namespace rodavarion::gui
