#define ENET_IMPLEMENTATION
#include "./network.h"

network::network() {
    assert(enet_initialize() == 0);
    players.reserve(MAX_PLAYERS);
    
    ENetAddress addy = {0};
    addy.host = ENET_HOST_ANY;
    addy.port = PORT;

    server = enet_host_create(&addy, MAX_PLAYERS, 2, 0, 0);

    assert(server != NULL);

    running = true;
    std::cout << "Server started.\n";
};

void network::process(){
    this->receiver = std::thread([this](){
        ENetEvent event;
        while(this->running){
            if(enet_host_service(server, &event, 1000) > 0){
                switch(event.type){
                case ENET_EVENT_TYPE_CONNECT: {
                    client* nc = new client{event.peer->address.host, event.peer->address.port};
                    nc->self = nc;
                    event.peer->data = (void*)nc;
                    players.emplace_back(nc);
                    // TODO: Send a snapshot of the current world
                    std::cout << "Player connected!\n";
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    players.erase(std::remove(players.begin(), players.end(), ((client*)event.peer->data)->self));
                    delete ((client*)event.peer->data)->self;
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