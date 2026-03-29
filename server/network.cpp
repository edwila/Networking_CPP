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

void network::disconnect(ENetPeer* peer, uint8_t reason, const std::string& msg){
    // TODO: add a check where if reason == 1 (kicked), send a packet informing them they are kicked, then actually call enet_peer_disconnect_later(peer, reason)
    if(reason == 1 && !msg.empty()){
        std::vector<uint8_t> packet = {2};
        packet.insert(packet.end(), msg.begin(), msg.end());

        send(peer, packet);
    }
    enet_peer_disconnect_later(peer, reason);
};

void network::check_hbs(const std::chrono::steady_clock::time_point& now, Syncer* sync){
    for(auto plr = players.begin(); plr != players.end();){
        client* c = plr->second;
        const uint32_t& e_id = plr->first;

        //std::cout << "State: " << c->peer->state << "\n>> ";

        if(c->peer->state == ENET_PEER_STATE_DISCONNECTED) {
            std::cout << "[CLEANUP] Player [" << entt::to_entity(e_id) << "] had an ungraceful disconnect. Cleaning up...\n>> ";
            delete c;
            sync->delete_entity(e_id);
            plr = players.erase(plr);
            std::cout << "Player " << entt::to_entity(e_id) << " left!\n>> ";
            continue;
        } else if(c->peer->state == ENET_PEER_STATE_CONNECTED){
            std::chrono::milliseconds dur = std::chrono::duration_cast<std::chrono::milliseconds>(now-c->heartbeat);

            if(dur >= std::chrono::milliseconds(MAX_HB)){
                std::cout << "Player [" << entt::to_entity(e_id) << "] has not pinged with a heartbeat for " << dur.count() << " ms. Forcing a disconnect.\n>> ";
                disconnect(c->peer, 1, "Lost connection: heartbeat ping not received.");
            }
        }

        plr++;
    }
};

void network::process(Syncer* sync){
    auto& buffer = sync->get_buffer();

    receiver = std::thread([&, sync](){
        ENetEvent event;
        while(running){
            while(enet_host_service(server, &event, 1000) > 0){
                switch(event.type){
                    case ENET_EVENT_TYPE_CONNECT: {
                        enet_peer_timeout(event.peer, ENET_PEER_TIMEOUT_LIMIT, 2000, 5000);

                        // Player connected, send a snapshot of all networked entities and create a new Player entity
                        std::vector<uint8_t> snapshot = sync->handshake_snapshot();

                        if(snapshot.size() > 1){
                            send(event.peer, snapshot);
                            snapshot = {1};
                            send(event.peer, snapshot); // Send 0 to denote handshaking as completed
                        }

                        entt::entity plr = sync->create_entity(); // Can initialize the entity here with player components
                        sync->add_tag<Networked>(plr);
                        uint32_t integral = entt::to_integral(plr);
                        client* nc = new client{};
                        nc->host = event.peer->address.host;
                        nc->port = event.peer->address.port;
                        nc->self = nc;
                        nc->peer = event.peer;
                        event.peer->data = (void*)(uintptr_t)integral;
                        players[integral] = nc;
                        // Sends a snapshot of the current world here
                        std::cout << "Player connected! Assigned entity index: " << entt::to_entity(integral) << " (" << integral << ")\n>> ";
                        break;
                    }
                    case ENET_EVENT_TYPE_DISCONNECT: {
                        uint32_t entity_id = (uint32_t)(uintptr_t)event.peer->data;
                        if(players.find(entity_id) == players.end()) {
                            std::cout << "[ERROR] Ghost Player! Entity " << entity_id << " disconnected but wasn't found in the map!\n";
                            break; // It bails out here and never erases them!
                        }

                        std::cout << "Player " << entt::to_entity(entity_id) << " left!\n>> ";

                        delete players[entity_id];
                        sync->delete_entity(entity_id);
                        players.erase(entity_id);
                        break;
                    }
                    case ENET_EVENT_TYPE_RECEIVE: {
                        if(event.packet->dataLength > 0){
                                switch(*(event.packet->data)){
                                    case 0: {
                                        // Heartbeat ping
                                        uint32_t e_id = (uint32_t)(uintptr_t)event.peer->data;
                                        client* current_client = players[e_id];

                                        if(current_client == nullptr){
                                            // Do something here, idk
                                            std::cout << "Attempted to get a heartbeat from non-existant entity! Returning...\n";
                                            return;
                                        }

                                        //std::cout << "Entity " << (e_id & 0xFFFFF) << " heartbeat.\n>> ";

                                        current_client->heartbeat = std::chrono::steady_clock::now();
                                        break;
                                    }
                                    case 1: {
                                        // Chat
                                        // Do filtering and stuff here
                                        // text is in event.packet->data+1 up to event.packet->dataLength
                                        for(size_t L = 1; L < event.packet->dataLength; L++){
                                            // 1 because 0 is the flag
                                            std::cout << (char)(*(event.packet->data+L));
                                        }
                                        std::cout << "\n";
                                        std::vector<uint8_t> packet_data = {3};
                                        uint32_t entity_id = (uint32_t)(uintptr_t)event.peer->data;

                                        uint8_t* ptr = (uint8_t*)&entity_id; // The entity ID (enet_uint32)
                                        packet_data.insert(packet_data.end(), ptr, ptr+(sizeof(enet_uint32)));
                                        packet_data.insert(packet_data.end(), event.packet->data+1, event.packet->data+event.packet->dataLength);
                                        send_to_all(packet_data);
                                        break;
                                    }
                                }
                            }
                            enet_packet_destroy(event.packet);
                        break;
                    }
                    case ENET_EVENT_TYPE_NONE: {
                        break;
                    }
                }
            }
            std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

            if(std::chrono::duration_cast<std::chrono::milliseconds>(now-last_hb) >= std::chrono::milliseconds(HB_TICKRATE)){
                // TODO: Designate a specific heartbeat thread here (instead of doing it in this loop)
                last_hb = now;
                check_hbs(now, sync);
            }
        }
    });
};

std::unordered_map<uint32_t, client*>& network::get_players(){
    return players;
};

void network::send(ENetPeer* receiver, std::vector<uint8_t>& data_buf_obj){
    uint8_t flag = data_buf_obj[0];

    //std::cout << "Flag: " << (int)flag << " sending as " << (FLAGS[flag] ? "RELIABLE" : "UNRELIABLE") << "\n";

    int res = enet_peer_send(receiver, 0, enet_packet_create((void*)data_buf_obj.data(), data_buf_obj.size(), ENET_PACKET_FLAG_RELIABLE));
    // todo: 
    // == 0: success | < 0: error
}

void network::send(ENetPeer* receiver, EventStream& data_buf_obj, bool use_mutex){
    if(use_mutex){
        data_buf_obj.lock();
    }
    this->send(receiver, data_buf_obj.data);
    if(use_mutex){
        data_buf_obj.unlock();
    }
}

void network::send_strict(std::vector<enet_uint32>& targets, std::vector<uint8_t>& data_buf_obj){
    for(enet_uint32 ent : targets){
        if(players.count(ent) > 0){
            // They're an entity we're currently tracking, so it's valid
            this->send(players[ent]->peer, data_buf_obj);
        }
    }
}

void network::send_strict(std::vector<enet_uint32>& targets, EventStream& data_buf_obj, bool use_mutex){
    if(use_mutex){
        data_buf_obj.lock();
    }
    this->send_strict(targets, data_buf_obj.data);
    if(use_mutex){
        data_buf_obj.unlock();
    }
}

void network::send_to_all(std::vector<uint8_t>& data_buf_obj){
    uint8_t flag = data_buf_obj[0];

    //std::cout << "Flag: " << (int)flag << " sending as " << (FLAGS[flag] ? "RELIABLE" : "UNRELIABLE") << "\n";
    
    enet_host_broadcast(server, 0, enet_packet_create((void*)data_buf_obj.data(), data_buf_obj.size(), ENET_PACKET_FLAG_RELIABLE));
};

void network::send_to_all(EventStream& data_buf_obj, bool use_mutex){
    if(use_mutex){
        data_buf_obj.lock();
    }
    this->send_to_all(data_buf_obj.data);
    if(use_mutex){
        data_buf_obj.unlock();
    }
};

void network::clean_up(){
    running = false;

    if(receiver.joinable()) receiver.join();

    if(server){
        enet_host_destroy(server);
        server = nullptr;
    }
    enet_deinitialize();

    std::cout << "Network process joined successfully.\n";
};

network::~network(){
    clean_up();
}