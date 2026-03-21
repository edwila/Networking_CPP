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

struct frame_tracker {
    uint64_t frame = 0;
};

#pragma pack(push, 1)
struct PacketHeader{
    uint32_t entity;
    uint32_t comp_id;
    uint8_t opcode;
};
#pragma pack(pop)

struct EventStream {
    std::mutex buffer_mutex;

    std::vector<uint8_t> data;

    frame_tracker* tracked = nullptr;

    EventStream(){
        data.reserve(BUFFER_SIZE);
    }

    void track(frame_tracker* tracker){
        tracked = tracker;
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

    std::vector<uint8_t> clone() const {
        return data;
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

    void write_event(entt::entity& entity, uint32_t comp_id, uint8_t opcode, const void* data = nullptr, size_t data_size = 0, bool use_mutex = false) {
        lock(use_mutex);
        
        // write exactly 9 bytes of header
        PacketHeader header = {entt::to_integral(entity), comp_id, opcode};
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&header);
        this->data.insert(this->data.end(), ptr, ptr + sizeof(PacketHeader));
        
        // write optional data
        if (data && data_size > 0) {
            const uint8_t* data_ptr = static_cast<const uint8_t*>(data);
            this->data.insert(this->data.end(), data_ptr, data_ptr + data_size);
        }
        
        unlock(use_mutex);
    }
};

#endif