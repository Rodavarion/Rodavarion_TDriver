#pragma once
#include <cstdint>
#include <string>
namespace rodavarion::catalog {
class VendorDatabase final {
public:
    [[nodiscard]] static std::string nameFor(std::uint16_t vendorId);
};
}
