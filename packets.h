#ifndef Packets_H
#define Packets_H

#include <string>
#include <any>
#include <cstdint>
#include <unordered_map>
#include <entt/entt.hpp>

enum OpCode { ADDED = 0, CHANGED = 1, REMOVED = 2 };

struct Packet {
    OpCode op;
    std::string component;
    std::any value;
};

using EventQueue = std::unordered_map<entt::entity, std::vector<Packet>>;

// Container for network packets
struct NetPackage {
    uint64_t frame_id;
    EventQueue queue;
};

#endif