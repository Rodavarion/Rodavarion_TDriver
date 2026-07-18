#pragma once

#include <string_view>

namespace rodavarion::capability {

enum class DeviceClass {
    Unknown,
    Mouse,
    Keyboard,
    Touchpad,
    Trackball,
    Gamepad,
    Joystick,
    SteeringWheel,
    BarcodeScanner,
    DocumentScanner,
    Printer,
    ThermalPrinter,
    LabelPrinter,
    ReceiptPrinter,
    Camera,
    Microphone,
    Headset,
    GraphicsTablet,
    Scale,
    Ups,
    Sensor,
    Receiver,
    Composite
};

enum class DeviceCapability {
    Pointer,
    Buttons,
    KeyboardInput,
    VerticalScroll,
    HorizontalScroll,
    Gestures,
    Axes,
    Triggers,
    ForceFeedback,
    Vibration,
    Calibration,
    BarcodeInput,
    SymbologySelection,
    PrefixSuffix,
    KeyboardWedgeMode,
    SerialMode,
    ImageScan,
    DuplexScan,
    Print,
    ColorPrint,
    DuplexPrint,
    PaperSize,
    PaperCut,
    CashDrawer,
    LabelMedia,
    PrinterStatus,
    EscPos,
    CameraCapture,
    AudioInput,
    AudioOutput,
    Battery,
    Profiles,
    ApplicationRules,
    Firmware,
    VendorProtocol,
    Diagnostics
};

[[nodiscard]] constexpr std::string_view
deviceClassName(const DeviceClass value) noexcept {
    switch (value) {
        case DeviceClass::Mouse:           return "Mouse";
        case DeviceClass::Keyboard:        return "Keyboard";
        case DeviceClass::Touchpad:        return "Touchpad";
        case DeviceClass::Trackball:       return "Trackball";
        case DeviceClass::Gamepad:         return "Gamepad";
        case DeviceClass::Joystick:        return "Joystick";
        case DeviceClass::SteeringWheel:   return "Steering wheel";
        case DeviceClass::BarcodeScanner:  return "Barcode scanner";
        case DeviceClass::DocumentScanner: return "Document scanner";
        case DeviceClass::Printer:         return "Printer";
        case DeviceClass::ThermalPrinter:  return "Thermal printer";
        case DeviceClass::LabelPrinter:    return "Label printer";
        case DeviceClass::ReceiptPrinter:  return "Receipt printer";
        case DeviceClass::Camera:          return "Camera";
        case DeviceClass::Microphone:      return "Microphone";
        case DeviceClass::Headset:         return "Headset";
        case DeviceClass::GraphicsTablet:  return "Graphics tablet";
        case DeviceClass::Scale:           return "Scale";
        case DeviceClass::Ups:             return "UPS";
        case DeviceClass::Sensor:          return "Sensor";
        case DeviceClass::Receiver:        return "Receiver";
        case DeviceClass::Composite:       return "Composite device";
        case DeviceClass::Unknown:         return "Unknown";
    }
    return "Unknown";
}

[[nodiscard]] constexpr std::string_view
capabilityName(const DeviceCapability value) noexcept {
    switch (value) {
        case DeviceCapability::Pointer:            return "Pointer";
        case DeviceCapability::Buttons:            return "Buttons";
        case DeviceCapability::KeyboardInput:      return "Keyboard input";
        case DeviceCapability::VerticalScroll:     return "Vertical scroll";
        case DeviceCapability::HorizontalScroll:   return "Horizontal scroll";
        case DeviceCapability::Gestures:           return "Gestures";
        case DeviceCapability::Axes:               return "Axes";
        case DeviceCapability::Triggers:           return "Triggers";
        case DeviceCapability::ForceFeedback:      return "Force feedback";
        case DeviceCapability::Vibration:          return "Vibration";
        case DeviceCapability::Calibration:        return "Calibration";
        case DeviceCapability::BarcodeInput:       return "Barcode input";
        case DeviceCapability::SymbologySelection: return "Symbologies";
        case DeviceCapability::PrefixSuffix:       return "Prefix / suffix";
        case DeviceCapability::KeyboardWedgeMode:  return "Keyboard-wedge mode";
        case DeviceCapability::SerialMode:         return "Serial mode";
        case DeviceCapability::ImageScan:          return "Image scan";
        case DeviceCapability::DuplexScan:         return "Duplex scan";
        case DeviceCapability::Print:              return "Print";
        case DeviceCapability::ColorPrint:         return "Color print";
        case DeviceCapability::DuplexPrint:        return "Duplex print";
        case DeviceCapability::PaperSize:          return "Paper size";
        case DeviceCapability::PaperCut:           return "Paper cut";
        case DeviceCapability::CashDrawer:         return "Cash drawer";
        case DeviceCapability::LabelMedia:         return "Label media";
        case DeviceCapability::PrinterStatus:      return "Printer status";
        case DeviceCapability::EscPos:             return "ESC/POS";
        case DeviceCapability::CameraCapture:      return "Camera capture";
        case DeviceCapability::AudioInput:         return "Audio input";
        case DeviceCapability::AudioOutput:        return "Audio output";
        case DeviceCapability::Battery:            return "Battery";
        case DeviceCapability::Profiles:           return "Profiles";
        case DeviceCapability::ApplicationRules:   return "Application rules";
        case DeviceCapability::Firmware:           return "Firmware";
        case DeviceCapability::VendorProtocol:     return "Vendor protocol";
        case DeviceCapability::Diagnostics:        return "Diagnostics";
    }
    return "Unknown";
}

} // namespace rodavarion::capability
