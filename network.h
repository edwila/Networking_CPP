#include "packets.h"
#include <vector>
#include <iostream>
#include <thread>
#include <algorithm>
#include "enet.h"

constexpr static int PORT = 6767;

constexpr static int MAX_PLAYERS = 10; // How many connections we'll allow

struct client {
    in6_addr host;
    enet_uint16 port;
    client* self;
};

// Create a class that'll handle all of the server's communication
class network {
private:
    std::thread receiver;
    ENetHost* server;

    bool running = false;

    std::vector<client*> players; // Keep track of connected players for rendering purposes later
public:
    network() {
        assert(enet_initialize() == 0);

        players.reserve(MAX_PLAYERS);
        
        ENetAddress addy = {0};

        addy.host = ENET_HOST_ANY;
        addy.port = PORT;

        server = enet_host_create(&addy, MAX_PLAYERS, 2, 0, 0);

        assert(server != NULL);

        running = true;

        std::cout << "Server started.\n";
    }

    void process(){
        // Accept stuff across the network boundary

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
                        // Send a snapshot of the current world
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
    }

    void send_to_all(const std::vector<uint8_t>& data_buf){
        // Send data_buf to all connected clients
        ENetPacket* packet = enet_packet_create((void*)data_buf.data(), data_buf.size(), ENET_PACKET_FLAG_RELIABLE);

        enet_host_broadcast(this->server, 0, packet);
    }

    void clean_up(){
        this->running = false;
        enet_host_destroy(server);
        enet_deinitialize();

        if(this->receiver.joinable()){
            this->receiver.join();
        }

        std::cout << "Network process joined successfully.\n";
    }

    ~network(){
        this->clean_up();
    }
};