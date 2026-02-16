#ifndef DECODER_H
#define DECODER_H

#include "../shared/components.h"
#include "../shared/packets.h"
#include <vector>
#include <cstring>
#include <iostream>

class Decoder {
public:
    Decoder(entt::registry& wrld) : world(wrld){};
    void write_to_buffer(const std::vector<uint8_t>& buf);
    void parse_and_erase();

private:
    std::vector<uint8_t> buffer;
    entt::registry& world;

    template <typename T>
    T parse(size_t& cursor){
        T val;
        if(cursor + sizeof(T) > buffer.size()){
            std::memset(&val, 0, sizeof(T));
        } else{
            std::memcpy(&val, buffer.data() + cursor, sizeof(T));
            cursor += sizeof(T);
        }
        return val;
    }
};

#endif