#ifndef _NETWORK_SERVER
#define _NETWORK_SERVER

#include "../shared/packets.h"
#include <vector>
#include <atomic>
#include <unordered_map>
#include <iostream>
#include <thread>
#include <algorithm>
#include "../shared/enet.h"

constexpr static uint32_t PORT = 6767;
constexpr static uint8_t MAX_PLAYERS = 10;

class Syncer;

// Struct to contain necessary client information (for a playerlist)
struct client {
    in6_addr host;
    enet_uint16 port;
    client* self;
    ENetPeer* peer;
};

// Struct to contain information about being kicked. Can be templated for different types of disconnects, leaving this as-is for simplicity
struct dc_packet {
    uint8_t dtype; // disconnection type

    void* metadata; // will be cast to the specific packet depending on dtype 
    size_t metadata_len;
};

struct kick_packet {
    std::string reason;
};

typedef std::unordered_map<uint32_t, client*> player_list;

class network {
public:
    network();
    void process(Syncer* sync);
    void send_to_all(EventStream& data_buf_obj, bool use_mutex = false);
    void send_to_all(std::vector<uint8_t>& data_buf_obj);
    void clean_up();
    void disconnect(ENetPeer* peer, uint8_t reason = 0);
    player_list get_players() const;
    ~network();
private:
    std::thread receiver;
    ENetHost* server;
    std::atomic<bool> running = false;
    player_list players; // Keep track of connected players for rendering and broadcasting purposes later
    // key = entityID of the Player
    // value = client class
};

#endif