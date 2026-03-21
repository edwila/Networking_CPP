#define ENET_IMPLEMENTATION

#include "./network.h"


network::network() {
    assert(enet_initialize() == 0);

    init();

    std::cout << "Client started.\n";
};

void network::init(){
    running = true;

    client = enet_host_create(NULL, 1, 2, 0, 0);

    assert(client != NULL);
}

void network::connect(std::string& hostIP, int counter){
    ENetAddress addy = {0};
    enet_address_set_host(&addy, hostIP.data());
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
        std::cout << "Connection failed. Attempting again... (" << counter << "/" << MAX_ATTEMPTS << ")\n";
        enet_peer_reset(peer);
        return connect(hostIP, counter++);
    }
};

bool network::is_connected() const {
    return connected;
};

void network::process(){
    receiver = std::thread([this]() {
        ENetEvent event;

        while (running) {
            if(connected){
                int serviceResult = enet_host_service(client, &event, 1000);

                if (serviceResult > 0) {
                    switch (event.type) {
                        case ENET_EVENT_TYPE_RECEIVE: {
                            std::cout << "\nPacket received: " << event.packet->dataLength << " bytes\n";
                            uint8_t* data_ptr = event.packet->data;
                            for(size_t i = 0; i < event.packet->dataLength; i++){
                                std::cout << static_cast<int>(*(data_ptr+i)) << " ";
                            }
                            std::cout << "\n>> ";
                            enet_packet_destroy(event.packet);
                            break;
                        }
                        case ENET_EVENT_TYPE_DISCONNECT: {
                            enet_uint32 dc_type = event.data;
                            switch (dc_type) {
                                default:
                                case 0: {
                                    std::cout << "\nLost connection to the server...\n>> ";
                                    // todo: attempt reconnection here
                                    break;
                                }
                                case 1: {
                                    std::cout << "\nYou've been kicked from the game!\n>> ";
                                    break;
                                }
                            }

                            connected = false;
                            peer = nullptr;

                            //running = false;
                            break;
                        }
                    }
                } else if (serviceResult < 0) {
                    std::cout << "Error with ENet service.\n";
                    //running = false;
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }

        if(connected){
            disconnect();
        }
    });
};

bool network::is_running() const {
    return running;
}

void network::disconnect(){
    if(peer){
        std::cout << "Ended connection.\n";
        enet_peer_disconnect(peer, 0);

        enet_host_flush(client);

        peer = nullptr;
        connected = false;
    }
};

void network::clean_up(){
    running = false;

    if(receiver.joinable()) receiver.join();

    if(client){
        enet_host_destroy(client);
        client = nullptr;
    }
    enet_deinitialize();

    std::cout << "Network process joined successfully.\n";
};

network::~network(){
    clean_up();
};