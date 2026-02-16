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
#include "../components.h"
#include "network.h"

class Syncer {
public:
    explicit Syncer(entt::registry* _world){

        if(_world == nullptr){
            world = std::make_unique<entt::registry>();
        } else{
            world.reset(_world);
        }
    };

    ~Syncer(){
        this->clean_up();
    }

    void clean_up(){
        this->running = false;

        this->updater.join();

        this->net.clean_up();

        std::cout << "Syncer clean up protocol completed. Exiting gracefully.\n";

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    uint64_t get_frame() const {
        return frame;
    };

    void set_frame(const uint64_t& fr){
        frame = fr;
    }

    entt::registry& get_world(){
        return *world;
    };

    bool initialized(){
        return world != nullptr;
    };

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

    entt::entity create_entity(){
        assert(initialized());

        return world->create();
    };

    void init(){
        // Create a new ENet connection and listen for stuff
        std::cout << "Client init.\n";

        net.process();
    }

    bool is_networked(entt::entity e){
        assert(initialized());

        return world->all_of<Networked>(e);
    }

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