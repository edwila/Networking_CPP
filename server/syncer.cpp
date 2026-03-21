#include "./syncer.h"

Syncer::Syncer(std::shared_ptr<entt::registry> _world)
    : world(_world ? _world : std::make_shared<entt::registry>())
{
    
};

Syncer::~Syncer(){
    clean_up();
};

void Syncer::clean_up(){
    running = false;
    if(updater.joinable()) updater.join();
    if(listener.joinable()) listener.join();
    net.clean_up();

    std::cout << "Syncer clean up protocol completed. Exiting gracefully.\n";

    std::this_thread::sleep_for(std::chrono::seconds(5));
};

void Syncer::increment_frame(){
    frame++;
};

uint64_t Syncer::get_frame() const {
    return frame;
};

entt::registry& Syncer::get_world(){
    return *world;
};

bool Syncer::initialized(){
    return world != nullptr;
};

entt::entity Syncer::create_entity(){
    assert(initialized());
    return world->create();
};

uint8_t Syncer::delete_entity(uint32_t e){
    if(!initialized()) return 1;
    entt::entity conv_e = static_cast<entt::entity>(e);
    if(!world->valid(conv_e)) return 2;
    
    world->destroy(conv_e);

    return 0;
};

void Syncer::init(){
    register_hooks<Postation>();
    register_hooks<Name>();

    updater = std::thread(&Syncer::update, this);
    listener = std::thread(&Syncer::listen, this);
};

void Syncer::listen(){
    net.process(this);
};

void Syncer::update(){
    // This way, we can simulate the framerate (1000 milliseconds in a second)
    // so 1000/framerate gives us how many times to run a frame in a second
    auto now = std::chrono::steady_clock::now();
    auto dur = std::chrono::microseconds(1000000/framerate);

    while(running){
        // Fire buffer.data to clients here
        if(!buffer.data.empty() && buffer.attempt_lock()){ // This will attempt to lock the mutex. It wont block the updater thread, however.
            // If we lock successfully, then we have the mutex so we can just send to all and clear without the use of a mutex (second arg)
            print_buffer();
            net.send_to_all(buffer);
            buffer.reset();

            buffer.unlock(); // TODO: Change to unique_lock (maybe use std::defer_lock)
        }

        increment_frame();

        now += dur;

        std::this_thread::sleep_until(now);
    }
};

bool Syncer::is_networked(entt::entity e){
    assert(initialized());
    return world->all_of<Networked>(e);
};

player_list Syncer::get_players() const {
    return net.get_players();
};

void Syncer::kick(uint32_t plr){
    client* data = get_players()[plr];

    net.disconnect(data->peer);
}

void Syncer::print_buffer(){
    buffer.out(std::cout);
    std::cout << "frame: " << get_frame() << "\n>> ";
};