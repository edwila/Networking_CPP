#include "../shared/packets.h"
#include <vector>
#include <iostream>
#include <thread>
#include <algorithm>
#include "../shared/enet.h"

constexpr static int PORT = 6767;
constexpr static int MAX_PLAYERS = 10;

// Struct to contain necessary client information (for a playerlist)
struct client {
    in6_addr host;
    enet_uint16 port;
    client* self;
};

class network {
public:
    network();
    void process();
    void send_to_all(const std::vector<uint8_t>& data_buf);
    void clean_up();
    ~network();
private:
    std::thread receiver;
    ENetHost* server;
    bool running = false;
    std::vector<client*> players; // Keep track of connected players for rendering purposes later
};