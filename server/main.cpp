#include "../shared/entt/components.h"
#include "../shared/packets.h"
#include "../server/syncer.h"

int main(){
    // World initialized here
    Syncer syncer(nullptr);

    syncer.init();

    std::string user_entry;

    uint32_t counter = 0;

    entt::entity ent;

    while(user_entry != "exit"){
        std::cout << ">> ";
        std::cin >> user_entry;

        if(user_entry == "upd"){
            if(!syncer.get_world().valid(ent)){
                ent = syncer.create_entity();
                syncer.add_tag<Networked>(ent);
                std::cout << "World was not initialized. Done now!\n";
            }

            std::cout << "Updating world counter\n";

            syncer.get_world().emplace_or_replace<Postation>(ent, counter);
            syncer.get_world().emplace_or_replace<Name>(ent, std::to_string(counter++));
        } else if (user_entry == "init"){
            std::cout << "init'ing world\n";

            ent = syncer.create_entity();
            syncer.add_tag<Networked>(ent);
        } else if(user_entry == "kick"){
            uint32_t peer_name;
            std::cin >> peer_name;
            std::getline(std::cin >> std::ws, user_entry);

            user_entry = user_entry.length() > 0 ? user_entry : "No reason provided.";

            player_list players = syncer.get_players();
            if(players.find(peer_name) == players.end()){
                std::cout << "Please provide a valid entity ID for the player to kick.\n";
                continue;
            }

            std::cout << "Kicking " << peer_name << " with reason: " << user_entry << "\n";

            syncer.kick(peer_name, user_entry);
        } else if(user_entry == "playerlist"){
            // print the playerlist
            player_list players = syncer.get_players();

            std::cout << "Player list:\n";

            for(auto const& entry : players){
                char str_addr[INET6_ADDRSTRLEN];
                client* c = entry.second;

                if(inet_ntop(AF_INET6, &(c->host), str_addr, INET6_ADDRSTRLEN) != nullptr){
                    std::cout << "[" << entry.first << "]: " << str_addr << "\n";
                } else{
                    std::cout << "[" << entry.first << "]: " << c << "\n";
                }
            }
        }
    }

    syncer.clean_up();
    
    return 0;
}