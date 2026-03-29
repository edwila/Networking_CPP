#ifndef _NETWORK_SERVER
#define _NETWORK_SERVER

#include "../shared/packets.h"
#include <vector>
#include <atomic>
#include <bitset>
#include <unordered_map>
#include <iostream>
#include <thread>
#include <algorithm>
#include "../shared/enet.h"


class Syncer;

// Struct to contain necessary client information (for a playerlist)
struct client {
    in6_addr host;
    ENetPeer* peer;
    client* self;
    std::chrono::steady_clock::time_point heartbeat = std::chrono::steady_clock::now();     // heartbeat that the client sends to the server to stay connected

    enet_uint16 port;
};

typedef std::unordered_map<uint32_t, client*> player_list;

class network {
public:
    network();
    void process(Syncer* sync);
    void send_to_all(EventStream& data_buf_obj, bool use_mutex = false);
    void send_to_all(std::vector<uint8_t>& data_buf_obj);
    void send_strict(std::vector<enet_uint32>& targets, EventStream& data_buf_obj, bool use_mutex = false); // send_strict will accept a vector of entity IDs associated with players to send packets strictly to them
    void send_strict(std::vector<enet_uint32>& targets, std::vector<uint8_t>& data_buf_obj);
    void send(ENetPeer* receiver, EventStream& data_buf_object, bool use_mutex);
    void send(ENetPeer* receiver, std::vector<uint8_t>& data_buf_object);
    void clean_up();
    void disconnect(ENetPeer* peer, uint8_t reason = 0, const std::string& msg = "");
    player_list& get_players();
    void check_hbs(const std::chrono::steady_clock::time_point& now, Syncer* sync);
    ~network();

    /*
    FLAGS indices (right -> left):
    0 - Syncing
    1 - Completed handshake protocol alert
    2 - Kicking a player
    3 - Chat
    */
    static constexpr std::bitset<4> FLAGS = 0b1110; // TODO: Adjust these as needed (for different flags). 0 = UNRELIABLE, 1 = RELIABLE
    static constexpr uint32_t PORT = 6767;
    static constexpr uint8_t MAX_PLAYERS = 10;
    static constexpr uint16_t HB_TICKRATE = 1000; // Check every player heartbeats at this rate (in milliseconds)
    static constexpr uint16_t MAX_HB = 5000; // If no heartbeat >= MAX_HB, force disconnect the player
private:
    std::thread receiver;
    ENetHost* server;
    std::atomic<bool> running = false;
    player_list players; // Keep track of connected players for rendering and broadcasting purposes later
    std::chrono::steady_clock::time_point last_hb = std::chrono::steady_clock::now();
    // key = entityID of the Player
    // value = client class
};

#endif