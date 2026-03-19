#ifndef _NETWORK_SERVER
#define _NETWORK_SERVER

#include "../shared/packets.h"
#include <vector>
#include <unordered_map>
#include <iostream>
#include <thread>
#include <algorithm>
#include "../shared/enet.h"

constexpr static int PORT = 6767;
constexpr static int MAX_PLAYERS = 10;

class Syncer;

// Struct to contain necessary client information (for a playerlist)
struct client {
    in6_addr host;
    enet_uint16 port;
    client* self;
};

class network {
public:
    network();
    void process(Syncer* sync);
    void send_to_all(const std::vector<uint8_t>& data_buf);
    void clean_up();
    ~network();
private:
    std::thread receiver;
    ENetHost* server;
    bool running = false;
    std::unordered_map<uint32_t, client*> players; // Keep track of connected players for rendering and broadcasting purposes later
    // key = entityID of the Player
    // value = client class
};

#endif