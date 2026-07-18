#include "rodavarion/hid/HidDeviceProvider.hpp"
#include "rodavarion/core/Logger.hpp"

#include <hidapi/hidapi.h>

#include <QString>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace rodavarion::hid {

namespace {

std::string fromWide(const wchar_t* value) {
    if (value == nullptr) {
        return {};
    }

    return QString::fromWCharArray(value).toStdString();
}

class HidDevice final : public device::IDevice {
public:
    explicit HidDevice(device::DeviceInfo info)
        : info_(std::move(info)) {}

    ~HidDevice() override {
        close();
    }

    [[nodiscard]] const device::DeviceInfo& info() const noexcept override {
        return info_;
    }

    [[nodiscard]] bool isConnected() const noexcept override {
        return handle_ != nullptr;
    }

    bool open() override {
        if (handle_ != nullptr) {
            return true;
        }

        if (info_.systemPath.empty()) {
            return false;
        }

        handle_ = hid_open_path(info_.systemPath.c_str());
        info_.connected = handle_ != nullptr;
        return handle_ != nullptr;
    }

    void close() noexcept override {
        if (handle_ != nullptr) {
            hid_close(handle_);
            handle_ = nullptr;
        }
        info_.connected = false;
    }

private:
    device::DeviceInfo info_;
    hid_device* handle_{nullptr};
};

} // namespace

HidDeviceProvider::HidDeviceProvider() {
    initialized_ = hid_init() == 0;

    if (!initialized_) {
        core::Logger::instance().log(
            core::LogLevel::Error,
            "HIDAPI initialization failed."
        );
    }
}

HidDeviceProvider::~HidDeviceProvider() {
    if (initialized_) {
        hid_exit();
    }
}

std::string_view HidDeviceProvider::name() const noexcept {
    return "HIDAPI";
}

std::vector<device::DeviceInfo> HidDeviceProvider::scan() {
    std::vector<device::DeviceInfo> result;

    if (!initialized_) {
        return result;
    }

    hid_device_info* devices = hid_enumerate(0x0, 0x0);
    hid_device_info* current = devices;

    while (current != nullptr) {
        device::DeviceInfo info;
        info.id.vendorId = current->vendor_id;
        info.id.productId = current->product_id;
        info.id.serialNumber = fromWide(current->serial_number);
        info.manufacturer = fromWide(current->manufacturer_string);
        info.productName = fromWide(current->product_string);
        info.systemPath = current->path != nullptr ? current->path : "";
        info.backendName = "HIDAPI";
        info.connection = device::ConnectionType::Usb;
        info.connected = true;

        if (info.manufacturer.empty()) {
            info.manufacturer = "Unknown manufacturer";
        }

        if (info.productName.empty()) {
            info.productName = "Unnamed HID device";
        }

        result.push_back(std::move(info));
        current = current->next;
    }

    hid_free_enumeration(devices);
    return result;
}

std::unique_ptr<device::IDevice>
HidDeviceProvider::create(const device::DeviceId& id) {
    for (const auto& info : scan()) {
        if (info.id == id) {
            return std::make_unique<HidDevice>(info);
        }
    }

    return {};
}

} // namespace rodavarion::hid
