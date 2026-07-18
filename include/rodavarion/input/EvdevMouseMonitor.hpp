#pragma once

#include "rodavarion/action/MouseAction.hpp"

#include <QObject>
#include <QMetaType>
#include <QString>
#include <QVector>

class QSocketNotifier;
struct libevdev;
struct libevdev_uinput;

namespace rodavarion::input {

struct EvdevDeviceInfo {
    QString path;
    QString name;
    bool readable{false};
    bool mouseCandidate{false};
    bool hasRelativeMotion{false};
    bool hasPrimaryButtons{false};
    bool hasSideButtons{false};
    int openError{0};
};

class EvdevMouseMonitor final : public QObject {
    Q_OBJECT

public:
    explicit EvdevMouseMonitor(QObject* parent = nullptr);
    ~EvdevMouseMonitor() override;

    [[nodiscard]] static QVector<EvdevDeviceInfo> enumerateInputDevices();
    [[nodiscard]] bool start(
        const QString& devicePath,
        bool exclusiveGrab = false,
        bool relayGrabbedEvents = false
    );
    void stop() noexcept;

    [[nodiscard]] bool isRunning() const noexcept;
    [[nodiscard]] QString currentDevicePath() const;

signals:
    void buttonEvent(rodavarion::action::MouseButton button, bool pressed, int linuxCode);
    void monitorError(const QString& message);
    void monitorStateChanged(bool running);

private:
    void readAvailableEvents();
    [[nodiscard]] static bool mapButtonCode(
        int linuxCode,
        rodavarion::action::MouseButton& button
    ) noexcept;

    int fileDescriptor_{-1};
    libevdev* device_{nullptr};
    QSocketNotifier* notifier_{nullptr};
    QString devicePath_;
    bool grabbed_{false};
    bool relayGrabbedEvents_{false};
    libevdev_uinput* virtualDevice_{nullptr};
};

} // namespace rodavarion::input

Q_DECLARE_METATYPE(rodavarion::action::MouseButton)
