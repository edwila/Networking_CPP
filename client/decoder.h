#ifndef DECODER_H
#define DECODER_H

#include "components.h"
#include "packets.h"
#include <vector>
#include <cstring>
#include <iostream>

class Decoder {
public:
    Decoder(entt::registry& wrld) : world(wrld){};
    void write_to_buffer(const std::vector<uint8_t>& buf){
        this->buffer = buf;
    }

    void parse_and_erase(){
        if(buffer.empty()) return;

        size_t cursor = 0;

        while(cursor < buffer.size()){
            if(cursor + sizeof(entt::entity) + 2 > buffer.size()) break;

            entt::entity e = parse<entt::entity>(cursor);

            uint8_t comp_id = parse<uint8_t>(cursor);

            uint8_t opcode = parse<uint8_t>(cursor);

            if(!world.valid(e)){
                auto& id = world.storage<entt::entity>();
                id.emplace(e);
            }

            switch(opcode){
                case OpCode::CHANGED:
                case OpCode::ADDED:{
                    switch(comp_id){
                        case COMP_IDS::COMP_NAME: {
                            uint8_t len = parse<uint8_t>(cursor);
                            
                            break;
                        }
                    }
                    break;
                }
            }
        }

        this->buffer.clear();
    }

private:
    std::vector<uint8_t> buffer;
    entt::registry& world;

    template <typename T>
    T parse(size_t& cursor){
        T val;
        if(cursor + sizeof(T) > buffer.size()){
            std::memset(&val, 0, sizeof(T));
        } else{
            std::memset(&val, buffer.data() + cursor, sizeof(T));
            cursor += sizeof(T);
        }

        return val;
    }
};

#endif