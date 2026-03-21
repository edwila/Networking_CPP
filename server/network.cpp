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

void network::disconnect(ENetPeer* peer){
    enet_peer_disconnect_now(peer, 0);
};

void network::process(Syncer* sync){
    receiver = std::thread([&, sync](){
        ENetEvent event;
        while(running){
            if(enet_host_service(server, &event, 1000) > 0){
                switch(event.type){
                case ENET_EVENT_TYPE_CONNECT: {
                    // Player connected, create a new Player entity
                    entt::entity plr = sync->create_entity();
                    uint32_t integral = entt::to_integral(plr);
                    client* nc = new client{event.peer->address.host, event.peer->address.port};
                    nc->self = nc;
                    nc->peer = event.peer;
                    event.peer->data = (void*)integral;
                    players[integral] = nc;
                    // Sends a snapshot of the current world here
                    std::cout << "Player connected! Assigned entity index: " << (integral & 0xFFFFF) << "\n>> ";
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    uint32_t entity_id = (uint64_t)event.peer->data;
                    if(players.find(entity_id) == players.end()) break;

                    std::cout << "Player " << entt::to_entity(entity_id) << " left!\n>> ";

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
    return players;
};

void network::send_to_all(EventStream& data_buf_obj, bool use_mutex){
    if(use_mutex){
        data_buf_obj.lock();
    }
    enet_host_broadcast(server, 0, enet_packet_create((void*)data_buf_obj.data.data(), data_buf_obj.data.size(), ENET_PACKET_FLAG_RELIABLE));
    if(use_mutex){
        data_buf_obj.unlock();
    }
};

void network::clean_up(){
    running = false;
    enet_host_destroy(server);
    enet_deinitialize();

    if(receiver.joinable()) receiver.join();

    std::cout << "Network process joined successfully.\n";
};

network::~network(){
    clean_up();
}