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
#include "../shared/packets.h"
#include "../shared/components.h"
#include "./network.h"

class Syncer {
public:
    explicit Syncer(entt::registry* _world);

    static uint8_t get_framerate(){
        return framerate;
    };

    void clean_up();
    void increment_frame();
    uint64_t get_frame() const;
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
    void listen();
    void update();

    template <typename T>
    void register_hooks(){
        assert(initialized());

        world->on_construct<T>().template connect<&Syncer::on_con<T>>(this);
        world->on_update<T>().template connect<&Syncer::on_upd<T>>(this);
        world->on_destroy<T>().template connect<&Syncer::on_rem<T>>(this);
    }

    bool is_networked(entt::entity e);
    void print_buffer();
    ~Syncer();
private:
    // buffer protocol: [ENTITY: 4 bytes] [COMP_ID: 4 bytes] [OPCODE: 1 byte] [DATA?: sizeof(COMP_ID)?]
    std::unique_ptr<entt::registry> world;
    uint64_t frame = 0;
    static constexpr uint8_t framerate = 1; // how many times this->update() is called per second
    EventStream buffer;
    network net;
    std::thread updater, listener;
    std::atomic<bool> running = true;

    template <typename T>
    void on_con(entt::registry& world, entt::entity e){
        buffer.write(e);
        buffer.write(T::COMP_ID);
        buffer.write(OpCode::ADDED);
        const auto& comp = world.get<T>(e);

        if constexpr (T::COMP_ID == COMP_IDS::COMP_NAME){
            const std::string& name = comp.value;
            uint8_t len = static_cast<uint8_t>(name.length());

            buffer.write(len);

            for(auto c : name){
                buffer.write(c);
            }
        } else{
            buffer.write(comp);
        }
    };

    template <typename T>
    void on_upd(entt::registry& world, entt::entity e){
        buffer.write(e);
        buffer.write(T::COMP_ID);
        buffer.write(OpCode::CHANGED);
        const auto& comp = world.get<T>(e);

        if constexpr (T::COMP_ID == COMP_IDS::COMP_NAME){
            const std::string& name = comp.value;
            uint8_t len = static_cast<uint8_t>(name.length());

            buffer.write(len);

            for(auto c : name){
                buffer.write(c);
            }
        } else{
            buffer.write(comp);
        }
    };

    template <typename T>
    void on_rem(entt::registry& world, entt::entity e){
        buffer.write(e);
        buffer.write(T::COMP_ID);
        buffer.write(OpCode::REMOVED);
    };
};

#endif