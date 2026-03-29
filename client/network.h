#include <vector>
#include <atomic>
#include <iostream>
#include <thread>
#include <bitset>
#include <algorithm>
#include "../shared/enet.h"
#include "../shared/entt/components.h"


class network {
public:
    network();
    void init();
    void process();
    void clean_up();
    void connect(std::string& hostIP, int counter = 1); // Uses counter to recursively call itself if it fails up to MAX_ATTEMPTS times
    void disconnect();
    bool is_connected() const;
    void heartbeat();
    void send(std::vector<uint8_t>& packet);
    bool is_running() const;
    ~network();
    
    /*
    FLAGS indices (right -> left):
    0 - Heartbeat (RELIABLE)
    1 - Chat (RELIABLE)
    2 - UNUSED
    3 - UNUSED
    */
    static constexpr std::bitset<4> FLAGS = 0b0011; // TODO: Adjust these as needed (for different flags). 0 = UNRELIABLE, 1 = RELIABLE
    static constexpr uint16_t PORT = 6767;
    static constexpr uint8_t MAX_ATTEMPTS = 3;
    static constexpr uint16_t HB_TICKRATE = 2000; // heartbeat tickrate (in milliseconds)
private:
    std::thread receiver;
    ENetHost* client;
    ENetPeer* peer = nullptr;
    std::atomic<bool> running = false, connected = false;
    std::chrono::steady_clock::time_point last_hb = std::chrono::steady_clock::now();
};