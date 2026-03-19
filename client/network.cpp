#define ENET_IMPLEMENTATION

#include "./network.h"


network::network() {
    assert(enet_initialize() == 0);

    this->init();

    std::cout << "Client started.\n";
};

void network::init(){
    running = true;

    client = enet_host_create(NULL, 1, 2, 0, 0);

    assert(client != NULL);
}

void network::connect(int counter){
    ENetAddress addy = {0};
    enet_address_set_host(&addy, "127.0.0.1");
    addy.port = PORT;

    peer = enet_host_connect(client, &addy, 2, 0);
    assert(peer != NULL);
    
    ENetEvent event;
                
    if (enet_host_service(client, &event, 5000) > 0 &&
    event.type == ENET_EVENT_TYPE_CONNECT) {            
        std::cout << "Connection successful.\n";
        connected = true;
    } else {
        if(counter >= MAX_ATTEMPTS){
            std::cout << "Attempted to re-establish connection " << counter << " times. Please check your network and try again.\n";
            enet_peer_reset(peer);
            return;
        }
        std::cout << "Connection failed. Attempting again... (" << counter << "/" << MAX_ATTEMPTS << ")";
        enet_peer_reset(peer);
        return this->connect(counter++);
    }
};

bool network::is_connected() const {
    return connected;
};

void network::process(){
    this->receiver = std::thread([this]() {
        ENetEvent event;

        while (this->running) {
            if(connected){
                int serviceResult = enet_host_service(client, &event, 1000);

                if (serviceResult > 0) {
                    switch (event.type) {
                        case ENET_EVENT_TYPE_RECEIVE:
                            std::cout << "Packet received: " << event.packet->dataLength << " bytes\n";
                            enet_packet_destroy(event.packet);
                            break;

                        case ENET_EVENT_TYPE_DISCONNECT:
                            std::cout << "Server disconnected.\n";
                            //this->running = false;
                            break;
                    }
                } else if (serviceResult < 0) {
                    std::cout << "Error with ENet service.\n";
                    //running = false;
                }
            }
        }

        disconnect();
    });
};

bool network::is_running() const {
    return running;
}

void network::disconnect(){
    std::cout << "Ended connection.\n";
    enet_peer_disconnect(peer, 0);

    connected = false;

    ENetEvent event;
        
    while (enet_host_service(this->client, &event, 5000) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                std::cout << "Disconnect succeeded.\n";
                return;
        }
    }
};

void network::clean_up(){
    this->running = false;
    disconnect();
    enet_host_destroy(client);
    enet_deinitialize();

    if(this->receiver.joinable()){
        this->receiver.join();
    }

    std::cout << "Network process joined successfully.\n";
};

network::~network(){
    this->clean_up();
};