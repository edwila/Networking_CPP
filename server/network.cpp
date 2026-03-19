#define ENET_IMPLEMENTATION
#include "./network.h"
#include "./syncer.h"

network::network() {
    assert(enet_initialize() == 0);
    
    ENetAddress addy = {0};
    addy.host = ENET_HOST_ANY;
    addy.port = PORT;

    server = enet_host_create(&addy, MAX_PLAYERS, 2, 0, 0);

    assert(server != NULL);

    running = true;
    std::cout << "Server started.\n";
};

void network::process(Syncer* sync){
    this->receiver = std::thread([&, sync](){
        ENetEvent event;
        while(this->running){
            if(enet_host_service(server, &event, 1000) > 0){
                switch(event.type){
                case ENET_EVENT_TYPE_CONNECT: {
                    // Player connected, create a new Player entity
                    entt::entity plr = sync->create_entity();
                    client* nc = new client{event.peer->address.host, event.peer->address.port};
                    nc->self = nc;
                    event.peer->data = (void*)entt::to_integral(plr);
                    players[entt::to_integral(plr)] = nc;
                    // Sends a snapshot of the current world here
                    std::cout << "Player connected! Assigned entity ID: " << entt::to_integral(plr) << "\n";
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    uint32_t entity_id = (uint64_t)event.peer->data;
                    if(players.find(entity_id) == players.end()) return;

                    std::cout << "Player " << entity_id << " left!\n";

                    delete players[entity_id];
                    sync->delete_entity(entity_id);
                    players.erase(entity_id);
                    break;
                }
                case ENET_EVENT_TYPE_NONE: {
                    break;
                }
                }
            }
        }

        enet_host_destroy(server);
        enet_deinitialize();
    });
};

std::unordered_map<uint32_t, client*> network::get_players() const {
    return this->players;
};

void network::send_to_all(const std::vector<uint8_t>& data_buf){
    enet_host_broadcast(this->server, 0, enet_packet_create((void*)data_buf.data(), data_buf.size(), ENET_PACKET_FLAG_RELIABLE));
};

void network::clean_up(){
    this->running = false;
    enet_host_destroy(server);
    enet_deinitialize();

    if(this->receiver.joinable()){
        this->receiver.join();
    }

    std::cout << "Network process joined successfully.\n";
};

network::~network(){
    this->clean_up();
}