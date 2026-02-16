#include <vector>
#include <iostream>
#include <thread>
#include <algorithm>
#include "../shared/enet.h"
#include "../shared/components.h"

constexpr static int PORT = 6767;

class network {
public:
    network();
    void process();
    void clean_up();
    ~network();
private:
    std::thread receiver;
    ENetHost* client;
    bool running = false;
};