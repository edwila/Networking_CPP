#ifndef syncer_server_h
#define syncer_server_h

#include <string>
#include <any>
#include <entt/entt.hpp>
#include <memory>
#include <thread>
#include <functional>
#include <atomic>
#include <chrono>
#include "../shared/components.h"
#include "./network.h"

class Syncer {
public:
    explicit Syncer(entt::registry*);
    void clean_up();
    uint64_t get_frame() const;
    void set_frame(const uint64_t& fr);
    entt::registry& get_world();
    bool initialized();

    template <typename T, typename K>
    void emplace(const entt::entity& e, const K& value){
        assert(initialized());

        world->emplace<T>(e, value);
    };

    template <typename T>
    void add_tag(const entt::entity& e){
        assert(initialized());

        world->emplace<T>(e);
    };

    entt::entity create_entity();
    void init();
    bool is_networked(entt::entity e);
    void rem(entt::registry& world, entt::entity e, uint8_t id);
    ~Syncer();
private:
    // buffer protocol: [ENTITY: 4 bytes] [COMP_ID: 4 bytes] [OPCODE: 1 byte] [DATA?: sizeof(COMP_ID)?]
    std::unique_ptr<entt::registry> world;
    uint64_t frame = 0;
    static constexpr uint8_t framerate = 1; // how many times this->update() is called per second
    network net;
    std::thread updater;
    std::atomic<bool> running = true;
};

#endif