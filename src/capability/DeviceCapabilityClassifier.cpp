#include "rodavarion/capability/DeviceCapabilityClassifier.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace rodavarion::capability {

namespace {

std::string lowerCopy(std::string value) {
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](const unsigned char character) {
            return static_cast<char>(std::tolower(character));
        }
    );
    return value;
}

bool containsAny(
    const std::string& text,
    const std::initializer_list<std::string>& values
) {
    for (const auto& value : values) {
        if (text.find(value) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void appendUnique(
    std::vector<DeviceCapability>& capabilities,
    const DeviceCapability capability
) {
    if (std::find(
            capabilities.begin(),
            capabilities.end(),
            capability
        ) == capabilities.end()) {
        capabilities.push_back(capability);
    }
}

} // namespace

DeviceClassification
DeviceCapabilityClassifier::classify(
    const device::PhysicalDevice& device
) {
    DeviceClassification result;

    const auto product = lowerCopy(device.productName);
    const auto manufacturer = lowerCopy(device.manufacturer);

    if (containsAny(product, {"barcode", "bar code", "scanner gun", "imager"})) {
        result.deviceClass = DeviceClass::BarcodeScanner;
        result.capabilities = {
            DeviceCapability::BarcodeInput,
            DeviceCapability::SymbologySelection,
            DeviceCapability::PrefixSuffix,
            DeviceCapability::KeyboardWedgeMode,
            DeviceCapability::SerialMode,
            DeviceCapability::Profiles,
            DeviceCapability::Diagnostics
        };
    } else if (containsAny(product, {"thermal", "receipt printer", "pos printer"})) {
        result.deviceClass = containsAny(product, {"receipt", "pos"})
            ? DeviceClass::ReceiptPrinter
            : DeviceClass::ThermalPrinter;
        result.capabilities = {
            DeviceCapability::Print,
            DeviceCapability::PaperSize,
            DeviceCapability::PaperCut,
            DeviceCapability::CashDrawer,
            DeviceCapability::PrinterStatus,
            DeviceCapability::EscPos,
            DeviceCapability::Profiles,
            DeviceCapability::Diagnostics
        };
    } else if (containsAny(product, {"label printer", "labelwriter", "zebra"})) {
        result.deviceClass = DeviceClass::LabelPrinter;
        result.capabilities = {
            DeviceCapability::Print,
            DeviceCapability::PaperSize,
            DeviceCapability::LabelMedia,
            DeviceCapability::PrinterStatus,
            DeviceCapability::Profiles,
            DeviceCapability::Diagnostics
        };
    } else if (containsAny(product, {"printer"})) {
        result.deviceClass = DeviceClass::Printer;
        result.capabilities = {
            DeviceCapability::Print,
            DeviceCapability::PaperSize,
            DeviceCapability::PrinterStatus,
            DeviceCapability::Profiles,
            DeviceCapability::Diagnostics
        };
    } else if (containsAny(product, {"document scanner", "flatbed", "sheetfed"})) {
        result.deviceClass = DeviceClass::DocumentScanner;
        result.capabilities = {
            DeviceCapability::ImageScan,
            DeviceCapability::Profiles,
            DeviceCapability::Diagnostics
        };
        if (containsAny(product, {"duplex"})) {
            appendUnique(
                result.capabilities,
                DeviceCapability::DuplexScan
            );
        }
    } else if (containsAny(product, {"hotas", "flight stick", "flightstick", "throttle quadrant", "yoke"})) {
        result.deviceClass = DeviceClass::FlightController;
        result.capabilities = {DeviceCapability::Buttons,DeviceCapability::Axes,DeviceCapability::Triggers,DeviceCapability::ForceFeedback,DeviceCapability::Calibration,DeviceCapability::Profiles,DeviceCapability::ApplicationRules,DeviceCapability::Diagnostics};
    } else if (containsAny(product, {"rudder", "pedal", "pedals"})) {
        result.deviceClass = DeviceClass::Pedals;
        result.capabilities = {DeviceCapability::Buttons,DeviceCapability::Axes,DeviceCapability::Calibration,DeviceCapability::Profiles,DeviceCapability::ApplicationRules,DeviceCapability::Diagnostics};
    } else if (containsAny(product, {"spacemouse", "space mouse", "space navigator", "3dconnexion"})) {
        result.deviceClass = DeviceClass::SpaceController;
        result.capabilities = {DeviceCapability::Pointer,DeviceCapability::Buttons,DeviceCapability::Axes,DeviceCapability::Calibration,DeviceCapability::Profiles,DeviceCapability::ApplicationRules,DeviceCapability::Diagnostics};
    } else if (containsAny(product, {"presenter", "presentation", "clicker", "spotlight"})) {
        result.deviceClass = DeviceClass::Presenter;
        result.capabilities = {DeviceCapability::Buttons,DeviceCapability::KeyboardInput,DeviceCapability::Profiles,DeviceCapability::ApplicationRules,DeviceCapability::Diagnostics};
    } else if (containsAny(product, {"remote control", "remote", "air mouse", "media remote", "rcu"})) {
        result.deviceClass = DeviceClass::RemoteControl;
        result.capabilities = {DeviceCapability::Pointer,DeviceCapability::Buttons,DeviceCapability::KeyboardInput,DeviceCapability::Profiles,DeviceCapability::ApplicationRules,DeviceCapability::Diagnostics};
    } else if (containsAny(product, {"arcade", "fight stick", "fightstick", "dance pad"})) {
        result.deviceClass = DeviceClass::ArcadeController;
        result.capabilities = {DeviceCapability::Buttons,DeviceCapability::Axes,DeviceCapability::Calibration,DeviceCapability::Profiles,DeviceCapability::ApplicationRules,DeviceCapability::Diagnostics};
    } else if (containsAny(product, {"gamepad", "controller", "dualshock", "dualsense", "xbox", "joy-con"})) {
        result.deviceClass = DeviceClass::Gamepad;
        result.capabilities = {
            DeviceCapability::Buttons,
            DeviceCapability::Axes,
            DeviceCapability::Triggers,
            DeviceCapability::Vibration,
            DeviceCapability::Calibration,
            DeviceCapability::Profiles,
            DeviceCapability::ApplicationRules,
            DeviceCapability::Diagnostics
        };
    } else if (containsAny(product, {"joystick", "joy stick"})) {
        result.deviceClass = DeviceClass::Joystick;
        result.capabilities = {
            DeviceCapability::Buttons,
            DeviceCapability::Axes,
            DeviceCapability::ForceFeedback,
            DeviceCapability::Calibration,
            DeviceCapability::Profiles,
            DeviceCapability::ApplicationRules,
            DeviceCapability::Diagnostics
        };
    } else if (containsAny(product, {"wheel", "racing wheel", "driving force", "sim wheel"})) {
        result.deviceClass = DeviceClass::SteeringWheel;
        result.capabilities = {
            DeviceCapability::Buttons,
            DeviceCapability::Axes,
            DeviceCapability::ForceFeedback,
            DeviceCapability::Calibration,
            DeviceCapability::Profiles,
            DeviceCapability::ApplicationRules,
            DeviceCapability::Diagnostics
        };
    } else if (containsAny(product, {"touchpad", "elan1200"})) {
        result.deviceClass = DeviceClass::Touchpad;
        result.capabilities = {
            DeviceCapability::Pointer,
            DeviceCapability::Buttons,
            DeviceCapability::VerticalScroll,
            DeviceCapability::HorizontalScroll,
            DeviceCapability::Gestures,
            DeviceCapability::Profiles,
            DeviceCapability::Diagnostics
        };
    } else if (containsAny(product, {"trackball"})) {
        result.deviceClass = DeviceClass::Trackball;
        result.capabilities = {
            DeviceCapability::Pointer,
            DeviceCapability::Buttons,
            DeviceCapability::VerticalScroll,
            DeviceCapability::Profiles,
            DeviceCapability::Diagnostics
        };
    } else if (containsAny(product, {"keyboard"})) {
        result.deviceClass = DeviceClass::Keyboard;
        result.capabilities = {
            DeviceCapability::KeyboardInput,
            DeviceCapability::Buttons,
            DeviceCapability::Profiles,
            DeviceCapability::ApplicationRules,
            DeviceCapability::Diagnostics
        };
    } else if (containsAny(product, {"receiver", "unifying", "bolt"})) {
        result.deviceClass = DeviceClass::Receiver;
        result.capabilities = {
            DeviceCapability::VendorProtocol,
            DeviceCapability::Diagnostics
        };
    } else if (containsAny(product, {"mouse", "master", "gladius", "aimpoint", "gaming mouse", "vertical mouse", "track mouse"})) {
        result.deviceClass = DeviceClass::Mouse;
        result.capabilities = {
            DeviceCapability::Pointer,
            DeviceCapability::Buttons,
            DeviceCapability::VerticalScroll,
            DeviceCapability::Profiles,
            DeviceCapability::ApplicationRules,
            DeviceCapability::Diagnostics
        };

        if (containsAny(product, {"master"})) {
            appendUnique(
                result.capabilities,
                DeviceCapability::HorizontalScroll
            );
            appendUnique(
                result.capabilities,
                DeviceCapability::Battery
            );
        }
    } else if (containsAny(product, {"camera", "webcam"})) {
        result.deviceClass = DeviceClass::Camera;
        result.capabilities = {
            DeviceCapability::CameraCapture,
            DeviceCapability::Profiles,
            DeviceCapability::Diagnostics
        };
    } else if (containsAny(product, {"microphone", "mic"})) {
        result.deviceClass = DeviceClass::Microphone;
        result.capabilities = {
            DeviceCapability::AudioInput,
            DeviceCapability::Profiles,
            DeviceCapability::Diagnostics
        };
    } else if (containsAny(product, {"headset", "headphone"})) {
        result.deviceClass = DeviceClass::Headset;
        result.capabilities = {
            DeviceCapability::AudioInput,
            DeviceCapability::AudioOutput,
            DeviceCapability::Profiles,
            DeviceCapability::Diagnostics
        };
    } else if (containsAny(product, {"tablet", "pen tablet"})) {
        result.deviceClass = DeviceClass::GraphicsTablet;
        result.capabilities = {
            DeviceCapability::Pointer,
            DeviceCapability::Buttons,
            DeviceCapability::Axes,
            DeviceCapability::Profiles,
            DeviceCapability::ApplicationRules,
            DeviceCapability::Diagnostics
        };
    } else if (containsAny(product, {"scale", "weigh"})) {
        result.deviceClass = DeviceClass::Scale;
        result.capabilities = {
            DeviceCapability::SerialMode,
            DeviceCapability::Profiles,
            DeviceCapability::Diagnostics
        };
    } else if (containsAny(product, {"ups", "uninterruptible"})) {
        result.deviceClass = DeviceClass::Ups;
        result.capabilities = {
            DeviceCapability::Battery,
            DeviceCapability::Diagnostics
        };
    } else if (device.interfaceCount() > 1) {
        result.deviceClass = DeviceClass::Composite;
        result.capabilities = {
            DeviceCapability::Profiles,
            DeviceCapability::Diagnostics
        };
    }

    if (containsAny(manufacturer, {"logitech", "asus", "asustek", "razer", "corsair", "steelseries", "thrustmaster", "fanatec", "moza", "3dconnexion"})) {
        appendUnique(
            result.capabilities,
            DeviceCapability::VendorProtocol
        );
    }

    return result;
}

} // namespace rodavarion::capability
