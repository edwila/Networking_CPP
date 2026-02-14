#include "components.h"
#include "packets.h"
#include "syncer_server.h"


int main(){
    // World initialized here

    Syncer sync(nullptr);

    sync.register_hooks<Postation>();
    sync.register_hooks<Name>();

    entt::registry& world = sync.get_world();

    auto entity = sync.create_entity();

    sync.emplace<Networked, bool>(entity, true);

    sync.emplace<Postation, uint32_t>(entity, 0xFFFFFFFF);

    sync.emplace<Name, std::string>(entity, "Ismail");

    sync.create_init_package();

    std::cout << "\nPress ENTER to exit...\n";

    std::cin.get();
    
    return 0;
}