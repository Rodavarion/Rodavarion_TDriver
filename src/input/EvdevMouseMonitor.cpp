#include "rodavarion/input/EvdevMouseMonitor.hpp"

#include <QDir>
#include <QFile>
#include <QSocketNotifier>

#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <linux/input-event-codes.h>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

namespace rodavarion::input {

namespace {

QString sysfsName(const QString& eventName) {
    QFile file(QString("/sys/class/input/%1/device/name").arg(eventName));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString::fromUtf8(file.readAll()).trimmed();
    }
    return {};
}

bool nameLooksLikeMouse(const QString& name) {
    const auto lower = name.toLower();
    return lower.contains("mouse")
        || lower.contains("mx master")
        || lower.contains("trackball")
        || lower.contains("touchpad")
        || lower.contains("logitech");
}

} // namespace

EvdevMouseMonitor::EvdevMouseMonitor(QObject* parent)
    : QObject(parent) {
    qRegisterMetaType<rodavarion::action::MouseButton>(
        "rodavarion::action::MouseButton"
    );
}

EvdevMouseMonitor::~EvdevMouseMonitor() {
    stop();
}

QVector<EvdevDeviceInfo> EvdevMouseMonitor::enumerateInputDevices() {
    QVector<EvdevDeviceInfo> result;
    QDir directory("/dev/input");

    const auto entries = directory.entryList(
        {"event*"},
        QDir::System | QDir::Files | QDir::NoDotAndDotDot,
        QDir::Name
    );

    for (const auto& entry : entries) {
        EvdevDeviceInfo info;
        info.path = directory.absoluteFilePath(entry);
        info.name = sysfsName(entry);
        if (info.name.isEmpty()) {
            info.name = entry;
        }
        info.mouseCandidate = nameLooksLikeMouse(info.name);

        const int descriptor = ::open(
            info.path.toLocal8Bit().constData(),
            O_RDONLY | O_NONBLOCK | O_CLOEXEC
        );

        if (descriptor < 0) {
            info.openError = errno;
            result.push_back(info);
            continue;
        }

        info.readable = true;

        libevdev* device = nullptr;
        const int code = libevdev_new_from_fd(descriptor, &device);

        if (code == 0 && device != nullptr) {
            const char* deviceName = libevdev_get_name(device);
            if (deviceName != nullptr && *deviceName != '\0') {
                info.name = QString::fromUtf8(deviceName);
            }

            info.hasPrimaryButtons =
                libevdev_has_event_code(device, EV_KEY, BTN_LEFT)
                || libevdev_has_event_code(device, EV_KEY, BTN_RIGHT)
                || libevdev_has_event_code(device, EV_KEY, BTN_MIDDLE);

            info.hasSideButtons =
                libevdev_has_event_code(device, EV_KEY, BTN_SIDE)
                || libevdev_has_event_code(device, EV_KEY, BTN_BACK)
                || libevdev_has_event_code(device, EV_KEY, BTN_EXTRA)
                || libevdev_has_event_code(device, EV_KEY, BTN_FORWARD)
                || libevdev_has_event_code(device, EV_KEY, BTN_TASK);

            info.hasRelativeMotion =
                libevdev_has_event_code(device, EV_REL, REL_X)
                || libevdev_has_event_code(device, EV_REL, REL_Y);

            info.mouseCandidate =
                info.hasPrimaryButtons
                || info.hasSideButtons
                || info.hasRelativeMotion
                || nameLooksLikeMouse(info.name);

            libevdev_free(device);
        }

        ::close(descriptor);
        result.push_back(info);
    }

    return result;
}

bool EvdevMouseMonitor::start(
    const QString& devicePath,
    const bool exclusiveGrab,
    const bool relayGrabbedEvents
) {
    stop();

    fileDescriptor_ = ::open(
        devicePath.toLocal8Bit().constData(),
        O_RDONLY | O_NONBLOCK | O_CLOEXEC
    );

    if (fileDescriptor_ < 0) {
        emit monitorError(
            QString("Cannot open %1 read-only: %2")
                .arg(devicePath, QString::fromLocal8Bit(std::strerror(errno)))
        );
        return false;
    }

    const int resultCode = libevdev_new_from_fd(fileDescriptor_, &device_);
    if (resultCode < 0 || device_ == nullptr) {
        emit monitorError(
            QString("libevdev cannot initialize %1: %2")
                .arg(devicePath, QString::fromLocal8Bit(std::strerror(-resultCode)))
        );
        stop();
        return false;
    }

    if (exclusiveGrab && relayGrabbedEvents) {
        const int virtualResult = libevdev_uinput_create_from_device(
            device_,
            LIBEVDEV_UINPUT_OPEN_MANAGED,
            &virtualDevice_
        );

        if (virtualResult < 0 || virtualDevice_ == nullptr) {
            emit monitorError(
                QString("Не вдалося створити безпечний віртуальний канал для %1: %2. Перевірте доступ до /dev/uinput.")
                    .arg(devicePath)
                    .arg(QString::fromLocal8Bit(std::strerror(-virtualResult)))
            );
            stop();
            return false;
        }

        relayGrabbedEvents_ = true;
    }

    if (exclusiveGrab) {
        const int grabResult = libevdev_grab(
            device_,
            LIBEVDEV_GRAB
        );

        if (grabResult < 0) {
            emit monitorError(
                QString("Не вдалося увімкнути ексклюзивне перехоплення %1: %2")
                    .arg(devicePath)
                    .arg(QString::fromLocal8Bit(
                        std::strerror(-grabResult)
                    ))
            );
            stop();
            return false;
        }

        grabbed_ = true;
    }

    devicePath_ = devicePath;
    notifier_ = new QSocketNotifier(
        fileDescriptor_,
        QSocketNotifier::Read,
        this
    );

    connect(notifier_, &QSocketNotifier::activated, this, [this]() {
        readAvailableEvents();
    });

    emit monitorStateChanged(true);
    return true;
}

void EvdevMouseMonitor::stop() noexcept {
    if (notifier_ != nullptr) {
        notifier_->setEnabled(false);
        delete notifier_;
        notifier_ = nullptr;
    }
    if (device_ != nullptr) {
        if (grabbed_) {
            libevdev_grab(device_, LIBEVDEV_UNGRAB);
            grabbed_ = false;
        }

        libevdev_free(device_);
        device_ = nullptr;
    }
    if (virtualDevice_ != nullptr) {
        libevdev_uinput_destroy(virtualDevice_);
        virtualDevice_ = nullptr;
    }
    relayGrabbedEvents_ = false;
    if (fileDescriptor_ >= 0) {
        ::close(fileDescriptor_);
        fileDescriptor_ = -1;
    }

    const bool wasRunning = !devicePath_.isEmpty();
    devicePath_.clear();
    if (wasRunning) {
        emit monitorStateChanged(false);
    }
}

bool EvdevMouseMonitor::isRunning() const noexcept {
    return fileDescriptor_ >= 0 && device_ != nullptr;
}

QString EvdevMouseMonitor::currentDevicePath() const {
    return devicePath_;
}

void EvdevMouseMonitor::readAvailableEvents() {
    if (device_ == nullptr) {
        return;
    }

    input_event event{};
    while (true) {
        const int code = libevdev_next_event(
            device_,
            LIBEVDEV_READ_FLAG_NORMAL,
            &event
        );

        if (code == -EAGAIN) {
            break;
        }
        if (code == LIBEVDEV_READ_STATUS_SYNC) {
            continue;
        }
        if (code < 0) {
            emit monitorError(
                QString("Read-only monitor error: %1")
                    .arg(QString::fromLocal8Bit(std::strerror(-code)))
            );
            stop();
            break;
        }
        bool suppressFromDesktop = false;
        action::MouseButton mappedButton{};
        const bool isMappedButton =
            event.type == EV_KEY
            && mapButtonCode(event.code, mappedButton);

        if (isMappedButton) {
            suppressFromDesktop =
                mappedButton == action::MouseButton::Back
                || mappedButton == action::MouseButton::Forward
                || mappedButton == action::MouseButton::Gesture
                || mappedButton == action::MouseButton::Extra1
                || mappedButton == action::MouseButton::Extra2;
        }

        if (relayGrabbedEvents_
            && virtualDevice_ != nullptr
            && !suppressFromDesktop) {
            const int relayResult = libevdev_uinput_write_event(
                virtualDevice_,
                event.type,
                event.code,
                event.value
            );

            if (relayResult < 0) {
                emit monitorError(
                    QString("Помилка ретрансляції події миші: %1")
                        .arg(QString::fromLocal8Bit(std::strerror(-relayResult)))
                );
            }
        }

        if (event.type == EV_KEY) {
            if (isMappedButton) {
                emit buttonEvent(mappedButton, event.value != 0, event.code);
            }
            continue;
        }

        if (event.type == EV_REL && event.value != 0) {
            action::MouseButton wheelButton;
            bool mapped = true;

            if (event.code == REL_WHEEL) {
                wheelButton = event.value > 0
                    ? action::MouseButton::WheelUp
                    : action::MouseButton::WheelDown;
            } else if (event.code == REL_HWHEEL) {
                wheelButton = event.value > 0
                    ? action::MouseButton::WheelRight
                    : action::MouseButton::WheelLeft;
            } else {
                mapped = false;
            }

            if (mapped) {
                emit buttonEvent(wheelButton, true, event.code);
                emit buttonEvent(wheelButton, false, event.code);
            }
        }
    }
}

bool EvdevMouseMonitor::mapButtonCode(
    const int linuxCode,
    action::MouseButton& button
) noexcept {
    switch (linuxCode) {
        case BTN_LEFT: button = action::MouseButton::Left; return true;
        case BTN_RIGHT: button = action::MouseButton::Right; return true;
        case BTN_MIDDLE: button = action::MouseButton::Middle; return true;
        case BTN_SIDE:
        case BTN_BACK: button = action::MouseButton::Back; return true;
        case BTN_EXTRA:
        case BTN_FORWARD: button = action::MouseButton::Forward; return true;
        case BTN_TASK: button = action::MouseButton::Gesture; return true;
        default: return false;
    }
}

} // namespace rodavarion::input
