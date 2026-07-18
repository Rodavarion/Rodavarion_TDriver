#include "rodavarion/capability/DeviceCapability.hpp"
#include "rodavarion/core/Application.hpp"
#include "rodavarion/core/Logger.hpp"
#include "rodavarion/runtime/MouseRuntimeController.hpp"
#include "rodavarion/context/ContextBridge.hpp"

#include <QCoreApplication>
#include <QTimer>
#include <QDBusConnection>

#include <exception>
#include <memory>
#include <vector>

namespace {

class HeadlessService final {
public:
    HeadlessService(
        rodavarion::core::Application& application,
        QObject* parent
    )
        : application_(application),
          runtime_(std::make_unique<
              rodavarion::runtime::MouseRuntimeController
          >(parent)) {}

    void start() {
        scanAndStart();

        timer_ = new QTimer(runtime_.get());
        timer_->setInterval(10000);

        QObject::connect(
            timer_,
            &QTimer::timeout,
            runtime_.get(),
            [this]() {
                if (!runtime_->isRunning()) {
                    scanAndStart();
                }
            }
        );

        timer_->start();
    }

private:
    void scanAndStart() {
        const auto peripherals =
            application_.scanPeripherals();

        for (const auto& peripheral : peripherals) {
            if (peripheral.classification.deviceClass
                    == rodavarion::capability::DeviceClass::Mouse
                && peripheral.physicalDevice.hasRealBackend()) {
                runtime_->start(peripheral);

                rodavarion::core::Logger::instance().log(
                    rodavarion::core::LogLevel::Info,
                    runtime_->statusText().toStdString()
                );
                return;
            }
        }

        rodavarion::core::Logger::instance().log(
            rodavarion::core::LogLevel::Warning,
            "Headless service is waiting for a supported mouse."
        );
    }

    rodavarion::core::Application& application_;
    std::unique_ptr<
        rodavarion::runtime::MouseRuntimeController
    > runtime_;
    QTimer* timer_{nullptr};
};

} // namespace

int main(int argc, char* argv[]) {
    QCoreApplication qtApplication(argc, argv);
    QCoreApplication::setApplicationName("Rodavarion TDriver");
    QCoreApplication::setApplicationVersion(QStringLiteral(RODAVARION_VERSION_STRING));
    QCoreApplication::setOrganizationName("Rodavarion Technologies");

    try {
        rodavarion::core::Application application;
        application.initialize();

        rodavarion::context::ContextBridge contextBridge(&qtApplication);
        auto sessionBus = QDBusConnection::sessionBus();
        if (!sessionBus.registerService("org.rodavarion.TDriver")
            || !sessionBus.registerObject(
                "/Context",
                &contextBridge,
                QDBusConnection::ExportAllSlots
            )) {
            rodavarion::core::Logger::instance().log(
                rodavarion::core::LogLevel::Warning,
                "Could not register the KWin context D-Bus bridge."
            );
        }

        HeadlessService service(application, &qtApplication);
        service.start();

        rodavarion::core::Logger::instance().log(
            rodavarion::core::LogLevel::Info,
            "Rodavarion TDriver headless service started."
        );

        return qtApplication.exec();
    } catch (const std::exception& error) {
        rodavarion::core::Logger::instance().log(
            rodavarion::core::LogLevel::Error,
            error.what()
        );
        return 1;
    } catch (...) {
        rodavarion::core::Logger::instance().log(
            rodavarion::core::LogLevel::Error,
            "Unknown fatal error in headless service."
        );
        return 2;
    }
}
