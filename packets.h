#ifndef Packets_H
#define Packets_H

#include <string>
#include <any>
#include <cstdint>
#include <unordered_map>
#include <iomanip>
#include <cctype>
#include <entt/entt.hpp>

constexpr size_t BUFFER_SIZE = 1024 * 1024;

enum OpCode : uint8_t { ADDED = 0, CHANGED = 1, REMOVED = 2 };

struct EventStream {
    std::vector<uint8_t> data; // Linear buffer

    EventStream(){
        data.reserve(BUFFER_SIZE);
    }

    void reset(){
        data.clear();
    }

    void out(std::ostream& s){
        s << "Buffer size: " << data.size() << " bytes\n";
    
        // Headers for each buffer entry will be 9 bytes. See syncer_server.h for why
        for (size_t i = 0; i < data.size(); i += 9) {

            for (size_t j = 0; j < 9; ++j) {
                if (i + j < data.size()) {
                    s <<  static_cast<int>(data[i + j]) << " ";
                } else {
                    s << "   ";
                }
            }
            
            s << "\n";
        }
    }

    template <typename T>
    void write(const T& value){
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&value);

        data.insert(data.end(), ptr, ptr + sizeof(T));
    }
};

#endif