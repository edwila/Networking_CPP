#include "./decoder.h"

void Decoder::write_to_buffer(const std::vector<uint8_t>& buf){
    this->buffer = buf;
};

void Decoder::parse_and_erase(){
    if(buffer.empty()) return;

    size_t cursor = 0;

    while(cursor < buffer.size()){
        if(cursor + sizeof(entt::entity) + 2 > buffer.size()) break;

        entt::entity e = parse<entt::entity>(cursor);
        uint8_t comp_id = parse<uint8_t>(cursor);
        uint8_t opcode = parse<uint8_t>(cursor);

        if(!world.valid(e)){
            auto& id = world.storage<entt::entity>();
            id.push(e);
        }

        switch(opcode){
            case OpCode::CHANGED:
            case OpCode::ADDED:{
                switch(comp_id){
                    case COMP_IDS::COMP_NAME: {
                        uint8_t len = parse<uint8_t>(cursor);
                        // TODO
                        break;
                    }
                }
                break;
            }
        }
    }

    this->buffer.clear();
}