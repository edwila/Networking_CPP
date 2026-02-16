#include <vector>
#include <iostream>
#include <thread>
#include <algorithm>
#include "../enet.h"
#include "../components.h"

constexpr static int PORT = 6767;

// Create a class that'll handle all of the server's communication
class network {
private:
    std::thread receiver;
    ENetHost* client;

    bool running = false;
public:
    network() {
        assert(enet_initialize() == 0);

        client = enet_host_create(NULL, 1, 2, 0, 0);

        assert(client != NULL);

        running = true;

        std::cout << "Client started.\n";
    }

    void process() {
        ENetAddress addy = {0};
        enet_address_set_host(&addy, "127.0.0.1");
        addy.port = PORT;

        ENetPeer* peer = enet_host_connect(client, &addy, 2, 0);
        assert(peer != NULL);

        this->receiver = std::thread([this, peer]() {
            ENetEvent event;
            
            if (enet_host_service(client, &event, 5000) > 0 &&
                event.type == ENET_EVENT_TYPE_CONNECT) {
                
                std::cout << "Connection successful.\n";
            } else {
                std::cout << "Connection failed :(.\n";
                enet_peer_reset(peer);
                return;
            }

            while (this->running) {
                int serviceResult = enet_host_service(client, &event, 1000);

                if (serviceResult > 0) {
                    switch (event.type) {
                        case ENET_EVENT_TYPE_RECEIVE:
                            std::cout << "Packet received: " << event.packet->dataLength << " bytes\n";
                            enet_packet_destroy(event.packet);
                            break;

                        case ENET_EVENT_TYPE_DISCONNECT:
                            std::cout << "Server disconnected.\n";
                            this->running = false;
                            break;
                    }
                } else if (serviceResult < 0) {
                    std::cout << "Error with ENet service.\n";
                    running = false;
                }
            }

            std::cout << "Ended connection.\n";
            enet_peer_disconnect(peer, 0);
            
            while (enet_host_service(client, &event, 3000) > 0) {
                switch (event.type) {
                    case ENET_EVENT_TYPE_RECEIVE:
                        enet_packet_destroy(event.packet);
                        break;
                    case ENET_EVENT_TYPE_DISCONNECT:
                        std::cout << "Disconnect succeeded.\n";
                        return;
                }
            }
        });
    }

    void clean_up(){
        enet_host_destroy(client);
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