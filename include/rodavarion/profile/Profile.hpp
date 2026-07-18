#pragma once

#include <map>
#include <string>

namespace rodavarion::profile {

struct Profile {
    std::string id;
    std::string displayName;
    std::map<std::string, std::string> settings;
};

} // namespace rodavarion::profile
