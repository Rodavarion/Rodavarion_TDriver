#include "rodavarion/runtime/MouseRuntimeController.hpp"

#include "rodavarion/action/MouseMappingStore.hpp"
#include "rodavarion/runtime/DesktopActionExecutor.hpp"
#include "rodavarion/input/MouseEventNormalizer.hpp"
#include "rodavarion/core/Logger.hpp"
#include "rodavarion/safety/ActionSafetyPolicy.hpp"


namespace rodavarion::runtime {

MouseRuntimeController::MouseRuntimeController(QObject* parent)
    : QObject(parent) {}

MouseRuntimeController::~MouseRuntimeController() {
    stop();
}

void MouseRuntimeController::start(
    const device::PeripheralDescriptor& peripheral
) {
    stop();

    peripheral_ = peripheral;
    hasPeripheral_ = true;

    const auto devices =
        input::EvdevMouseMonitor::enumerateInputDevices();

    action::MouseMappingStore mappingStore;
    const auto mappings = mappingStore.load(
        action::MouseMappingStore::defaultFilePath(),
        peripheral_.physicalDevice.key
    );

    bool needsSideButtonSuppression = false;
    if (mappings) {
        for (const auto& mapping : mappings.value().mappings) {
            const bool sideButton =
                mapping.button == action::MouseButton::Back
                || mapping.button == action::MouseButton::Forward
                || mapping.button == action::MouseButton::Gesture
                || mapping.button == action::MouseButton::Extra1
                || mapping.button == action::MouseButton::Extra2;

            const bool remapped =
                mapping.action != action::ActionType::Default
                && mapping.action != action::ActionType::Disabled;

            if (sideButton && remapped) {
                needsSideButtonSuppression = true;
                break;
            }
        }
    }

    int started = 0;
    int exclusiveStarted = 0;
    bool unsafeSharedInterfaceFound = false;

    for (const auto& device : devices) {
        if (!matches(device)) {
            continue;
        }

        const bool sharedMouseInterface =
            device.hasSideButtons
            && (device.hasRelativeMotion || device.hasPrimaryButtons);

        const bool exclusive =
            needsSideButtonSuppression
            && device.hasSideButtons;

        const bool relayGrabbedEvents =
            exclusive && sharedMouseInterface;

        if (relayGrabbedEvents) {
            unsafeSharedInterfaceFound = true;
        }

        auto monitor =
            std::make_unique<input::EvdevMouseMonitor>();

        connect(
            monitor.get(),
            &input::EvdevMouseMonitor::buttonEvent,
            this,
            &MouseRuntimeController::handleButton
        );

        connect(
            monitor.get(),
            &input::EvdevMouseMonitor::monitorError,
            this,
            [this](const QString& message) {
                status_ = message;
                core::Logger::instance().log(
                    core::LogLevel::Error,
                    message.toStdString()
                );
            }
        );

        if (monitor->start(device.path, exclusive, relayGrabbedEvents)) {
            ++started;
            if (exclusive) {
                ++exclusiveStarted;
            }
            monitors_.push_back(std::move(monitor));
        }
    }

    if (started == 0) {
        status_ = "Виконавчий модуль неактивний: немає доступного сумісного інтерфейсу миші.";
    } else if (needsSideButtonSuppression && exclusiveStarted == 0) {
        status_ = "Дії активні, але ексклюзивне перехоплення бічних кнопок не запущено.";
    } else {
        status_ = QString(
            "Виконавчий модуль активний: інтерфейсів %1; "
            "ексклюзивних інтерфейсів %2; режим ретрансляції: %3; бекенд: %4"
        )
            .arg(started)
            .arg(exclusiveStarted)
            .arg(unsafeSharedInterfaceFound ? "так" : "ні")
            .arg(DesktopActionExecutor::backendDescription());
    }
}

void MouseRuntimeController::stop() noexcept {
    for (auto& monitor : monitors_) {
        if (monitor) {
            monitor->stop();
        }
    }

    monitors_.clear();
    hasPeripheral_ = false;
    status_ = "Виконавчий модуль зупинено.";
}

bool MouseRuntimeController::isRunning() const noexcept {
    return !monitors_.empty();
}

QString MouseRuntimeController::statusText() const {
    return status_;
}

bool MouseRuntimeController::matches(
    const input::EvdevDeviceInfo& device
) const {
    if (!hasPeripheral_
        || !device.readable
        || !device.mouseCandidate) {
        return false;
    }

    const auto inputName = device.name.toLower();
    const auto productName = QString::fromStdString(
        peripheral_.physicalDevice.productName
    ).toLower();
    const auto manufacturer = QString::fromStdString(
        peripheral_.physicalDevice.manufacturer
    ).toLower();

    if (manufacturer.contains("logitech")
        || productName.contains("master")) {
        return inputName.contains("logitech")
            || inputName.contains("mx master");
    }

    return inputName.contains("mouse")
        || inputName.contains("trackball");
}

void MouseRuntimeController::handleButton(
    const action::MouseButton button,
    const bool pressed,
    const int nativeCode
) {
    if (!hasPeripheral_) {
        return;
    }

    const auto event = input::MouseEventNormalizer::normalize(
        button,
        pressed,
        nativeCode
    );

    if (event.kind == input::InputEventKind::Released) {
        return;
    }

    // Dispatch immediately while the focused application and the physical
    // button event still belong to the same input transaction. Delaying this
    // used to create races with context detection and could select the global
    // Ctrl+V shortcut instead of the terminal profile.
    const auto result = actionEngine_.dispatch(peripheral_, event);
    status_ = result.message;

    core::Logger::instance().log(
        result.success ? core::LogLevel::Info : core::LogLevel::Error,
        QString("Результат Action Engine: %1")
            .arg(result.message)
            .toStdString()
    );
}

} // namespace rodavarion::runtime
