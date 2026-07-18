#pragma once
#include "rodavarion/profile/Profile.hpp"
#include <optional>
#include <string_view>
#include <vector>
namespace rodavarion::profile {
class ProfileManager final {
public:
 bool add(Profile profile); bool update(Profile profile); void replaceAll(std::vector<Profile> profiles);
 [[nodiscard]] std::optional<Profile> find(std::string_view id) const;
 [[nodiscard]] const std::vector<Profile>& all() const noexcept;
private: std::vector<Profile> profiles_;
};
}
