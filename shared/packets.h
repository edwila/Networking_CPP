#ifndef Packets_H
#define Packets_H

#include <string>
#include <any>
#include <cstdint>
#include <unordered_map>
#include <mutex>
#include <iomanip>
#include <cctype>
#include <entt/entt.hpp>

// TODO: Make the buffer circular to not block when writing to it

constexpr size_t BUFFER_SIZE = 1024 * 1024;

enum OpCode : uint8_t { ADDED = 0, CHANGED = 1, REMOVED = 2 };

struct EventStream {
    std::mutex buffer_mutex;

    std::vector<uint8_t> data;

    EventStream(){
        data.reserve(BUFFER_SIZE);
    }

    void reset(bool use_mutex = false){
        lock(use_mutex); // Lock and unlock functions factor in use_mutex to determine whether they should actually try to lock

        data.clear();

        unlock(use_mutex);
    }

    bool attempt_lock(){
        // Try to lock (non-blocking)

        return buffer_mutex.try_lock();
    }

    void lock(bool use_mutex = true){
        // Attempt to lock the mutex (blocking)

        if(use_mutex){
            return buffer_mutex.lock();
        }
    }

    void unlock(bool use_mutex = true){
        // Attempt to unlock the mutex

        if(use_mutex){
            return buffer_mutex.unlock();
        }
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
    void write(const T& value, bool use_mutex = false){
        // TODO: Don't need to lock here, just use dual pointers to indicate a circular buffers
        lock(use_mutex);

        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&value);
        data.insert(data.end(), ptr, ptr + sizeof(T));
        
        unlock(use_mutex);
    }
};

#endif