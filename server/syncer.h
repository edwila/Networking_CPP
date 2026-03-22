#ifndef syncer_server_h
#define syncer_server_h

#include <string>
#include <any>
#include <mutex>
#include <entt/entt.hpp>
#include <memory>
#include <thread>
#include <functional>
#include <atomic>
#include <chrono>
#include "../shared/packets.h"
#include "../shared/entt/components.h"
#include "./network.h"

class Syncer {
public:
    explicit Syncer(std::shared_ptr<entt::registry> _world);

    static uint8_t get_framerate(){
        return framerate;
    };

    std::vector<uint8_t> handshake_snapshot();

    void clean_up();
    void kick(uint32_t plr, std::string& reason);
    void increment_frame();
    uint64_t get_frame() const;
    entt::registry& get_world();
    bool initialized();

    EventStream& get_buffer();

    player_list get_players() const;

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
    uint8_t delete_entity(uint32_t e); // return value of 0 = no errors, otherwise return value = error code
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

    template <typename T>
    void gen_payload(std::vector<uint8_t>& payload, entt::entity& e, T& comp){
        // Will fill the payload vector with the data needed to construct a delta packet (prepends with 0 to denote a sync packet)
        uint8_t* byte_ptr = (uint8_t*)&comp.value;
        size_t end_range = sizeof(comp.value);

        if constexpr (T::COMP_ID == COMP_IDS::COMP_NAME){
            const std::string& name = comp.value;
            uint8_t len = static_cast<uint8_t>(name.length());

            payload.reserve(payload.capacity()+len+1);
            payload.emplace_back(len);
            end_range = len;
            byte_ptr = (uint8_t*)name.data();
        }

        payload.insert(payload.end(), byte_ptr, byte_ptr+end_range);
    }
private:
    // buffer protocol: [ENTITY: 4 bytes] [COMP_ID: 4 bytes] [OPCODE: 1 byte] [DATA?: sizeof(COMP_ID)?]
    std::shared_ptr<entt::registry> world;
    frame_tracker frame;
    static constexpr uint8_t framerate = 1; // how many times this->update() is called per second
    EventStream buffer;
    network net;
    std::thread updater, listener;
    std::atomic<bool> running = true;

    template <typename T>
    void con_upd(entt::registry& world, entt::entity e, OpCode op){
        const auto& comp = world.get<T>(e);

        std::vector<uint8_t> payload;
        
        gen_payload(payload, e, comp);

        buffer.write_event(e, T::COMP_ID, op, payload.data(), payload.size(), true);
    };

    template <typename T>
    void on_con(entt::registry& world, entt::entity e){
        con_upd<T>(world, e, OpCode::ADDED);
    };

    template <typename T>
    void on_upd(entt::registry& world, entt::entity e){
        con_upd<T>(world, e, OpCode::CHANGED);
    };

    template <typename T>
    void on_rem(entt::registry& world, entt::entity e){
        buffer.write_event(e, T::COMP_ID, OpCode::REMOVED, nullptr, 0, true);
    };
};

#endif