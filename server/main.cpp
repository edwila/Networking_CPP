#include "../shared/components.h"
#include "../shared/packets.h"
#include "../server/syncer.h"

int main(){
    // World initialized here
    Syncer syncer(nullptr);

    syncer.init();

    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "Updating world now.\n";

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