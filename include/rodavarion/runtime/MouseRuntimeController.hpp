#pragma once

#include "rodavarion/device/PeripheralDescriptor.hpp"
#include "rodavarion/input/EvdevMouseMonitor.hpp"
#include "rodavarion/runtime/ActionEngine.hpp"
#include "rodavarion/runtime/DesktopOutputEngine.hpp"

#include <QObject>

#include <memory>
#include <vector>

namespace rodavarion::runtime {

class MouseRuntimeController final : public QObject {
public:
    explicit MouseRuntimeController(QObject* parent = nullptr);
    ~MouseRuntimeController() override;

    void start(const device::PeripheralDescriptor& peripheral);
    void stop() noexcept;

    [[nodiscard]] bool isRunning() const noexcept;
    [[nodiscard]] QString statusText() const;

private:
    void handleButton(
        action::MouseButton button,
        bool pressed,
        int linuxCode
    );

    [[nodiscard]] bool matches(
        const input::EvdevDeviceInfo& device
    ) const;

    device::PeripheralDescriptor peripheral_;
    bool hasPeripheral_{false};
    QString status_;

    DesktopOutputEngine outputEngine_;
    ActionEngine actionEngine_{outputEngine_};
    std::vector<std::unique_ptr<input::EvdevMouseMonitor>> monitors_;
};

} // namespace rodavarion::runtime
