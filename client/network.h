#include <vector>
#include <atomic>
#include <iostream>
#include <thread>
#include <bitset>
#include <algorithm>
#include "../shared/enet.h"
#include "../shared/entt/components.h"

constexpr static uint16_t PORT = 6767;
constexpr static uint8_t MAX_ATTEMPTS = 3;

/*
FLAGS indices:
0 - 
1 - Chat
2 - 
3 - 
*/
constexpr static std::bitset<4> FLAGS(0b1110); // TODO: Adjust these as needed (for different flags). 0 = UNRELIABLE, 1 = RELIABLE

class network {
public:
    network();
    void init();
    void process();
    void clean_up();
    void connect(std::string& hostIP, int counter = 0); // Uses counter to recursively call itself if it fails up to MAX_ATTEMPTS times
    void disconnect();
    bool is_connected() const;
    void send(std::vector<uint8_t>& packet);
    bool is_running() const;
    ~network();
private:
    std::thread receiver;
    ENetHost* client;
    ENetPeer* peer = nullptr;
    std::atomic<bool> running = false, connected = false;
};