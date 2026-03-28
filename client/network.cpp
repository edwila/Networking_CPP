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

    std::cout << "Connecting to: " << hostIP << "\n";

    peer = enet_host_connect(client, &addy, 2, 0); // TODO: send data instead of 0 (playerid, etc.)
    assert(peer != NULL);
    
    ENetEvent event;
                
    if (enet_host_service(client, &event, 5000) > 0 &&
    event.type == ENET_EVENT_TYPE_CONNECT) {            
        std::cout << "Connection successful.\n";
        connected = true;
    } else {
        if(counter > MAX_ATTEMPTS){
            std::cout << "Attempted to establish connection to " << hostIP << " " << counter << " times. Please check your network and try again.\n";
            enet_peer_reset(peer);
            return;
        }
        std::cout << "Connection failed. Attempting again... (" << counter << "/" << (int)MAX_ATTEMPTS << ")\n";
        enet_peer_reset(peer);
        return connect(hostIP, ++counter);
    }
};

bool network::is_connected() const {
    return connected;
};

void network::send(std::vector<uint8_t>& packet){
    // TODO: change flag based on the key of the packet (packet[0])
    uint8_t flag = packet[0];

    std::cout << "Flag: " << (int)flag << " sending as " << (FLAGS[flag] ? "RELIABLE" : "UNRELIABLE") << "\n";

    enet_peer_send(peer, 0, enet_packet_create((void*)packet.data(), packet.size(), FLAGS[flag] ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT));
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
                            if(event.packet->dataLength > 0){
                                switch(*(event.packet->data)){
                                    case 0: {
                                        std::cout << "\nSync packet received: " << event.packet->dataLength << " bytes (excluding flag byte)\n";
                                        uint8_t* data_ptr = event.packet->data;
                                        for(size_t i = 1; i < event.packet->dataLength; i++){
                                            std::cout << static_cast<int>(*(data_ptr+i)) << " ";
                                        }
                                        std::cout << "\n>> ";
                                        break;
                                    }
                                    case 1: {
                                        std::cout << "\nCompleted handshake protocol packet received: " << event.packet->dataLength << "\n";
                                        uint8_t* data_ptr = event.packet->data;
                                        for(size_t i = 0; i < event.packet->dataLength; i++){
                                            std::cout << static_cast<int>(*(data_ptr+i)) << " ";
                                        }
                                        std::cout << "\n>> ";
                                        break;
                                    }
                                    case 2: {
                                        std::string reason = "";
                                        uint8_t* data_ptr = event.packet->data;
                                        for(size_t i = 1; i < event.packet->dataLength; i++){
                                            reason +=  static_cast<char>(*(data_ptr+i));
                                        }

                                        std::cout << "\nYou've been kicked from the game: " << reason << "\n";
                                        break;
                                    }
                                    case 3: {
                                        uint8_t* data_ptr = event.packet->data;
                                        uint32_t entity_id;
                                        std::memcpy(&entity_id, data_ptr + 1, sizeof(uint32_t));
                                        std::string msg = "";
                                        for(size_t i = 1+sizeof(uint32_t); i < event.packet->dataLength; i++){
                                            msg +=  static_cast<char>(*(data_ptr+i));
                                        }

                                        std::cout << "\n[" << (entity_id & 0xFFFFF) << "]: " << msg << "\n>> ";
                                        break;
                                    }
                                }
                            }
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
                                    std::cout << "\nDisconnected.\n>> ";
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
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
        std::cout << "Ended connection.\n>> ";
        enet_peer_disconnect(peer, 0); // TODO: Disconnection data here
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