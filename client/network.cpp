#define ENET_IMPLEMENTATION

#include "./network.h"


network::network() {
    assert(enet_initialize() == 0);

    init();

    out("Client started.");
};

void network::heartbeat() {
    // If connected, send a heartbeat ping to the server
    if(is_connected()){
        std::vector<uint8_t> hb_packet = {0};
        send(hb_packet);
    }
}

void network::init(){
    running = true;

    client = enet_host_create(NULL, 1, 2, 0, 0);

    assert(client != NULL);
}

void network::connect(std::string& hostIP, int counter){
    ENetAddress addy = {0};
    enet_address_set_host(&addy, hostIP.data());
    addy.port = PORT;

    out("Connecting to ", hostIP);

    peer = enet_host_connect(client, &addy, 2, 0); // TODO: send data instead of 0 (playerid, etc.)
    assert(peer != NULL);
    
    ENetEvent event;
                
    if (enet_host_service(client, &event, 5000) > 0 &&
    event.type == ENET_EVENT_TYPE_CONNECT) { 
        out("Connection successful.");           
        connected = true;
    } else {
        if(counter > MAX_ATTEMPTS){
            out("Attempted to establish connection to ", hostIP, " ", counter, " times. Please check your network and try again.");
            enet_peer_reset(peer);
            return;
        }
        out("Connection failed. Attempting again... (", counter, "/", (int)MAX_ATTEMPTS, ")");
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

    //out("Flag: ", (int)flag, " sending as ", (FLAGS[flag] ? "RELIABLE" : "UNRELIABLE"));

    enet_peer_send(peer, 0, enet_packet_create((void*)packet.data(), packet.size(), FLAGS[flag] ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED));
};

void network::process(){
    receiver = std::thread([this]() {
        ENetEvent event;

        while (is_running()) {
            if(is_connected()){
                int serviceResult = enet_host_service(client, &event, 1000);

                if (serviceResult > 0) {
                    switch (event.type) {
                        case ENET_EVENT_TYPE_RECEIVE: {
                            if(event.packet->dataLength > 0){
                                switch(*(event.packet->data)){
                                    case 0: {
                                        out("Sync packet received: ", event.packet->dataLength, " bytes (excluding flag byte)");
                                        uint8_t* data_ptr = event.packet->data;
                                        std::string congregate = "";
                                        for(size_t i = 1; i < event.packet->dataLength; i++){
                                            congregate += std::to_string(static_cast<int>(*(data_ptr+i)));
                                            congregate += ' ';
                                        }
                                        out(congregate);
                                        break;
                                    }
                                    case 1: {
                                        out("Completed handshake protocol packet received!");
                                        uint8_t* data_ptr = event.packet->data;
                                        std::string congregate = "";
                                        for(size_t i = 0; i < event.packet->dataLength; i++){
                                            congregate += std::to_string(static_cast<int>(*(data_ptr+i)));
                                            congregate += ' ';
                                        }
                                        out(congregate);
                                        break;
                                    }
                                    case 2: {
                                        std::string reason = "";
                                        uint8_t* data_ptr = event.packet->data;
                                        for(size_t i = 1; i < event.packet->dataLength; i++){
                                            reason +=  static_cast<char>(*(data_ptr+i));
                                        }

                                        out("You've been kicked from the game: ", reason);
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

                                        out("[", entt::to_entity(entity_id), "]: ", msg);
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
                                    out("Lost connection to the server...");
                                    // todo: attempt reconnection here
                                    break;
                                }
                                case 1: {
                                    out("Disconnected.");
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
                    out("Error with ENet service.");
                    //running = false;
                }

                std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

                if(std::chrono::duration_cast<std::chrono::milliseconds>(now - last_hb) >= std::chrono::milliseconds(HB_TICKRATE)){
                    // Send a heartbeat ping
                    heartbeat();
                    last_hb = now;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }

        if(is_connected()){
            disconnect();
        }
    });
};

bool network::is_running() const {
    return running;
}

void network::disconnect(){
    if(peer){
        out("Ended connection.");
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

    out("Network process joined successfully.");
};

network::~network(){
    clean_up();
};