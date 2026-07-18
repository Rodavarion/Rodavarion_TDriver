#pragma once
#include "rodavarion/core/Result.hpp"
#include "rodavarion/profile/Profile.hpp"
#include <QString>
#include <vector>
namespace rodavarion::profile {
class ProfileStore final {
public:
 [[nodiscard]] static QString defaultFilePath();
 [[nodiscard]] core::Result<std::vector<Profile>> load(const QString&) const;
 [[nodiscard]] core::Result<void> save(const QString&,const std::vector<Profile>&) const;
};
}
