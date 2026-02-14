#ifndef syncer_server_h
#define syncer_server_h

#include <string>
#include <any>
#include <entt/entt.hpp>
#include <memory>
#include "packets.h"
#include "components.h"
#include "BinaryOutput.h"

class Syncer {
private:
    std::unique_ptr<entt::registry> _ptr;
    entt::registry* world = nullptr;
    uint64_t frame = 0;
    EventQueue events;    

public:
    Syncer(entt::registry* _world){

        if(_world == nullptr){
            _ptr = std::make_unique<entt::registry>();
            world = _ptr.get();
        } else{
            world = _world;
        }

    };

    void increment_frame(){
        frame++;
    };

    uint64_t get_frame() const {
        return frame;
    };

    void append(entt::entity e, Packet packet){
        events[e].push_back(packet);
    };

    EventQueue get_events() const {
        return events;
    };
    
    entt::registry* get_world(){
        return world;
    };

    bool initialized(){
        return world != nullptr;
    };

    template <typename T, typename K>
    void emplace(const entt::entity& e, const K& value){
        assert(initialized());

        world->emplace<T>(e, value);
    };

    void clear_events(){
        events.clear();
    };

    entt::entity create_entity(){
        assert(initialized());

        return world->create();
    };

    NetPackage create_package(){
        return NetPackage{get_frame(), get_events()};
    };

    EventQueue create_init_package(){
        assert(initialized());

        EventQueue package;

        BinaryOut output(std::cout);

        entt::snapshot{*world}.get<Networked>(output);

        return package;
    };

private:
    template <typename T>
    void on_con(entt::registry& world, entt::entity e){
        std::cout << "Hooked on a component construction!\n";
        T& comp = world.get<T>(e);
        std::cout << "Value of component: " << comp.value << "\n";
    };
    template <typename T>
    void on_upd(entt::registry& world, entt::entity e){
        std::cout << "Hooked on a component update!\n";
    };
    template <typename T>
    void on_rem(entt::registry& world, entt::entity e){
        std::cout << "Hooked on a component destroy!\n";
    };

public:
    template <typename T>
    void register_hooks(){
        assert(initialized());

        world->on_construct<T>().template connect<&Syncer::on_con<T>>(this);
        world->on_update<T>().template connect<&Syncer::on_upd<T>>(this);
        world->on_destroy<T>().template connect<&Syncer::on_rem<T>>(this);
    }
};

#endif