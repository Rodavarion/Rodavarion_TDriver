#include "rodavarion/profile/ProfileManager.hpp"
#include <algorithm>
#include <utility>
namespace rodavarion::profile {
bool ProfileManager::add(Profile p){if(p.id.empty())return false;auto it=std::find_if(profiles_.begin(),profiles_.end(),[&](const auto&i){return i.id==p.id;});if(it!=profiles_.end())return false;profiles_.push_back(std::move(p));return true;}
bool ProfileManager::update(Profile p){auto it=std::find_if(profiles_.begin(),profiles_.end(),[&](const auto&i){return i.id==p.id;});if(it==profiles_.end())return false;*it=std::move(p);return true;}
void ProfileManager::replaceAll(std::vector<Profile> p){profiles_=std::move(p);}
std::optional<Profile> ProfileManager::find(std::string_view id) const{auto it=std::find_if(profiles_.begin(),profiles_.end(),[&](const auto&i){return i.id==id;});return it==profiles_.end()?std::nullopt:std::optional<Profile>(*it);}
const std::vector<Profile>& ProfileManager::all() const noexcept{return profiles_;}
}
