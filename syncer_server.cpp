#include "components.h"
#include "packets.h"
#include "syncer_server.h"

int main(){
    // World initialized here

    WSAData wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    std::cout << "HEADER\n";

    Syncer syncer(nullptr);

    std::cout << "AFTER\n";

    syncer.init();

    std::cout << "INIT\n";

    entt::registry& world = syncer.get_world();

    auto other_entity = syncer.create_entity();

    auto entity = syncer.create_entity();

    syncer.add_tag<Networked>(entity);

    syncer.emplace<Postation, uint32_t>(entity, 0xFFFFFFFF);

    syncer.emplace<Name, std::string>(entity, "Ismail");

    syncer.get_world().emplace_or_replace<Name>(entity, "genscript");

    std::string user_entry;

    while(user_entry != "1"){
        std::cout << "Option: ";
        std::cin >> user_entry;
    }
    
    return 0;
}