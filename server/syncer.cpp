#include "./syncer.h"

Syncer::Syncer(entt::registry* _world){
    if(_world == nullptr){
        world = std::make_unique<entt::registry>();
    } else{
        world.reset(_world);
    }
};

Syncer::~Syncer(){
    this->clean_up();
};

void Syncer::clean_up(){
    this->running = false;
    this->updater.join();
    this->listener.join();
    this->net.clean_up();

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
    if(conv_e == entt::null) return 2;
    
    world->destroy(conv_e);

    return 0;
};

void Syncer::init(){
    this->register_hooks<Postation>();
    this->register_hooks<Name>();

    updater = std::thread(&Syncer::update, this);
    listener = std::thread(&Syncer::listen, this);
};

void Syncer::listen(){
    this->net.process(this);
};

void Syncer::update(){
    // This way, we can simulate the framerate (1000 milliseconds in a second)
    // so 1000/framerate gives us how many times to run a frame in a second
    std::cout << "update\n";
    while(this->running){
        // Fire buffer.data to clients here
        if(!buffer.data.empty()){
            this->print_buffer();
            net.send_to_all(buffer.data);
            buffer.reset();
        }
        this->increment_frame();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000/this->framerate));
    }
};

bool Syncer::is_networked(entt::entity e){
    assert(initialized());
    return world->all_of<Networked>(e);
};

player_list Syncer::get_players() const {
    return this->net.get_players();
};

void Syncer::print_buffer(){
    buffer.out(std::cout);
    std::cout << "frame: " << this->get_frame() << "\n";
};