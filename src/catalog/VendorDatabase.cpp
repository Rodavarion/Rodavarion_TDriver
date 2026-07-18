#include "rodavarion/catalog/VendorDatabase.hpp"
#include <unordered_map>
namespace rodavarion::catalog {
std::string VendorDatabase::nameFor(const std::uint16_t id) {
    static const std::unordered_map<std::uint16_t,std::string> db{
        {0x046D,"Logitech"},{0x04F3,"ELAN Microelectronics"},{0x045E,"Microsoft"},
        {0x1532,"Razer"},{0x1B1C,"Corsair"},{0x1038,"SteelSeries"},
        {0x0B05,"ASUSTek"},{0x413C,"Dell"},{0x17EF,"Lenovo"},
        {0x03F0,"HP"},{0x05AC,"Apple"},{0x1209,"Rodavarion Lab"}
    };
    const auto it=db.find(id); return it==db.end()?std::string{}:it->second;
}
}
