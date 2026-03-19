#include <vector>
#include <iostream>
#include <thread>
#include <algorithm>
#include "../shared/enet.h"
#include "../shared/entt/components.h"

constexpr static int PORT = 6767;

class network {
public:
    network();
    void process();
    void clean_up();
    void disconnect();
    ~network();
private:
    std::thread receiver;
    ENetHost* client;
    ENetPeer* peer;
    bool running = false;
};