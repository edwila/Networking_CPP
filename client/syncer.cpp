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
    this->net.clean_up();

    std::cout << "Syncer clean up protocol completed. Exiting gracefully.\n";

    std::this_thread::sleep_for(std::chrono::seconds(5));
};

uint64_t Syncer::get_frame() const {
    return frame;
};

void Syncer::set_frame(const uint64_t& fr){
    frame = fr;
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

void Syncer::init(){
    // Create a new ENet connection and listen for stuff
    std::cout << "Client init.\n";

    net.process();
};

bool Syncer::is_networked(entt::entity e){
    assert(initialized());

    return world->all_of<Networked>(e);
};

void Syncer::rem(entt::registry& world, entt::entity e, uint8_t id){
    switch(id){
        case COMP_IDS::COMP_NET: {
            world.remove<Networked>(e);
            break;
        }
        case COMP_IDS::COMP_NAME: {
            world.remove<Name>(e);
            break;
        }
        case COMP_IDS::COMP_POSTATION: {
            world.remove<Postation>(e);
            break;
        }
        default: {
            std::cout << "Unknown COMP_ID when removing.\n";
            break;
        }
    }
};