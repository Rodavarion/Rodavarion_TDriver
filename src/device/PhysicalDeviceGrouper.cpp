#include "rodavarion/device/PhysicalDeviceGrouper.hpp"
#include "rodavarion/catalog/VendorDatabase.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <unordered_map>
namespace rodavarion::device {
namespace {
std::string hex4(std::uint16_t v){std::ostringstream s;s<<std::uppercase<<std::hex<<std::setw(4)<<std::setfill('0')<<v;return s.str();}
std::string maker(const DeviceInfo& i){
 if(!i.manufacturer.empty()&&i.manufacturer!="Unknown manufacturer") return i.manufacturer;
 auto n=catalog::VendorDatabase::nameFor(i.id.vendorId); return n.empty()?"Unknown manufacturer":n;
}
std::string key(const DeviceInfo&i){
 auto base=hex4(i.id.vendorId)+":"+hex4(i.id.productId)+":";
 return base+(!i.id.serialNumber.empty()?i.id.serialNumber:i.productName+":"+maker(i));
}
}
std::vector<PhysicalDevice> PhysicalDeviceGrouper::group(const std::vector<DeviceInfo>& input){
 std::vector<PhysicalDevice> out; std::unordered_map<std::string,std::size_t> idx;
 for(auto i:input){ i.manufacturer=maker(i); auto k=key(i); auto it=idx.find(k);
  if(it==idx.end()){PhysicalDevice d{k,i.manufacturer,i.productName,{}};d.interfaces.push_back(std::move(i));idx[k]=out.size();out.push_back(std::move(d));}
  else out[it->second].interfaces.push_back(std::move(i));
 }
 std::sort(out.begin(),out.end(),[](const auto&a,const auto&b){return a.manufacturer==b.manufacturer?a.productName<b.productName:a.manufacturer<b.manufacturer;});
 return out;
}
}
