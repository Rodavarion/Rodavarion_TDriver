#pragma once

#include "rodavarion/action/MouseAction.hpp"
#include "rodavarion/device/PeripheralDescriptor.hpp"
#include "rodavarion/input/InputEvent.hpp"
#include "rodavarion/runtime/IOutputEngine.hpp"

#include <QDateTime>
#include <QString>

#include <optional>
#include <string>

namespace rodavarion::runtime {

class ActionEngine final {
public:
    explicit ActionEngine(IOutputEngine& outputEngine);
    ActionEngine(IOutputEngine& outputEngine, QString mappingsFilePath);

    [[nodiscard]] OutputResult dispatch(
        const device::PeripheralDescriptor& peripheral,
        const input::InputEvent& event
    );

private:
    [[nodiscard]] static std::optional<action::MouseButton> mappingButton(
        input::DeviceControl control
    ) noexcept;

    [[nodiscard]] bool refreshCacheIfNeeded(
        const std::string& deviceKey,
        QString& errorMessage
    );

    IOutputEngine& outputEngine_;
    QString mappingsFilePath_;
    std::optional<action::MouseMappingProfile> cachedProfile_;
    std::string cachedDeviceKey_;
    QDateTime cachedLastModified_;
    qint64 cachedFileSize_{-1};
    bool cachedFileExists_{false};
};

} // namespace rodavarion::runtime
