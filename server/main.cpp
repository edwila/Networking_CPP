#include "../shared/entt/components.h"
#include "../shared/packets.h"
#include "../server/syncer.h"

int main(){
    // World initialized here
    Syncer syncer(nullptr);

    syncer.init();

    std::string user_entry;

    while(user_entry != "exit"){
        std::cout << "Option: ";
        std::cin >> user_entry;

        if(user_entry == "modify"){
            std::cout << "Updating world now.\n";

            entt::registry& world = syncer.get_world();

            auto other_entity = syncer.create_entity();

            auto entity = syncer.create_entity();

            syncer.add_tag<Networked>(entity);

            syncer.emplace<Postation, uint32_t>(entity, 0xFFFFFFFF);

            syncer.emplace<Name, std::string>(entity, "Ismail");

            syncer.get_world().emplace_or_replace<Name>(entity, "genscript");
        } else if(user_entry == "playerlist"){
            // print the playerlist
            player_list players = syncer.get_players();

            std::cout << "Player list:\n";

            for(auto const& entry : players){
                char str_addr[INET6_ADDRSTRLEN];
                client* c = (client*)entry.second;

                if(inet_ntop(AF_INET6, &(c->host), str_addr, INET6_ADDRSTRLEN) != nullptr){
                    std::cout << "[" << entry.first << "]: " << str_addr << "\n";
                } else{
                    std::cout << "[" << entry.first << "]: " << entry.second << "\n";
                }
            }
        }
    }

    syncer.clean_up();
    
    return 0;
}