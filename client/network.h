#include <vector>
#include <atomic>
#include <iostream>
#include <thread>
#include <algorithm>
#include "../shared/enet.h"
#include "../shared/entt/components.h"

constexpr static uint16_t PORT = 6767;
constexpr static uint8_t MAX_ATTEMPTS = 3;

class network {
public:
    network();
    void init();
    void process();
    void clean_up();
    void connect(int counter = 0); // Uses counter to recursively call itself if it fails up to MAX_ATTEMPTS times
    void disconnect();
    bool is_connected() const;
    bool is_running() const;
    ~network();
private:
    std::thread receiver;
    ENetHost* client;
    ENetPeer* peer = nullptr;
    std::atomic<bool> running = false, connected = false;
};