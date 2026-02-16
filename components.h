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
#include <cstring>

constexpr size_t MAX_ENTITIES = 1000000; // 1 million entities for now

enum COMP_IDS : uint8_t { COMP_NET, COMP_NAME, COMP_POSTATION };

struct Networked {
    static constexpr uint8_t COMP_ID = COMP_IDS::COMP_NET;
};

struct Name {
    std::string value;

    Name(const std::string& str){
        value = str;
    }

    std::string get(){
        return value;
    }

    size_t get_size() const {
        return sizeof(value);
    }

    static constexpr uint8_t COMP_ID = COMP_IDS::COMP_NAME;
};

struct Postation {
    std::uint32_t packed;

    void set(std::uint32_t pos, uint16_t rot){
        packed = (rot & 0x1FF) | (pos << 9);
    }

    std::uint32_t get(){
        return packed;
    }

    uint32_t get_position() const {
        return (packed >> 9);
    }

    uint16_t get_rotation() const {
        return (packed & 0x1FF);
    }

    size_t get_size() const {
        return sizeof(packed);
    }

    static constexpr uint8_t COMP_ID = COMP_IDS::COMP_POSTATION;
};

#endif