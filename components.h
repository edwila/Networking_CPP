#ifndef Components_H
#define Components_H

// Components synced by the syncer
// Uses typelist in order to construct all our possible components
// For the sake of publicly releasing this code, I will only have 3 components:

// Networked : Boolean - Is this entity networked and synced through the syncer?
// Postation : uint32_t - Position + rotation encoded into a single 32bit number: [0:8] = rotation (0-360) [9:31] = rotation (assuming a top-down view of the map with a grid overlay)

#include <entt/entt.hpp>
#include <iostream>
#include <string>

struct Networked {
    bool value;
};

struct Name {
    std::string value;
};

struct Postation {
    uint32_t value;
};

// Will register the hooks to the component (on_construct, on_update, and on_destroy)

#endif