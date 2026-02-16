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
#include "packets.h"
#include "components.h"
#include "BinaryOutput.h"
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

    static uint8_t get_framerate(){
        return framerate;
    }

    void clean_up(){
        this->running = false;

        this->updater.join();
        this->listener.join();

        this->net.clean_up();

        std::cout << "Syncer clean up protocol completed. Exiting gracefully.\n";

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    void increment_frame(){
        frame++;
    };

    uint64_t get_frame() const {
        return frame;
    };

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
        this->register_hooks<Postation>();
        this->register_hooks<Name>();

        // Start the thread of updating via this->update();

        updater = std::thread(&Syncer::update, this);
        listener = std::thread(&Syncer::listen, this);
    }

    void listen(){
        this->net.process();
    }

    void update(){
        // This way, we can simulate the framerate (1000 milliseconds in a second)
        // so 1000/framerate gives us how many times to run a frame in a second

        std::cout << "update\n";
        while(this->running){
            this->print_buffer();
            this->increment_frame();

            // Fire buffer.data to clients here

            if(!buffer.data.empty()){
                net.send_to_all(buffer.data);
                buffer.reset();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1000/this->framerate));
        }
    }

    template <typename T>
    void register_hooks(){
        assert(initialized());

        world->on_construct<T>().template connect<&Syncer::on_con<T>>(this);
        world->on_update<T>().template connect<&Syncer::on_upd<T>>(this);
        world->on_destroy<T>().template connect<&Syncer::on_rem<T>>(this);
    }

    bool is_networked(entt::entity e){
        assert(initialized());

        return world->all_of<Networked>(e);
    }

    void print_buffer(){
        // Print the buffer
        buffer.out(std::cout);
        std::cout << "frame: " << this->get_frame() << "\n";
    }

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