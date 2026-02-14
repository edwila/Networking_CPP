#ifndef BinOut_H
#define BinOut_H

#include <iostream>
#include <fstream>
#include <type_traits>

struct BinaryOut {
    std::ostream& stream;

    explicit BinaryOut(std::ostream& s) : stream(s) {}

    template<typename T>
    void operator()(const T& data) {
        static_assert(std::is_trivially_copyable_v<T>, "Too complex :(");

        const char* ptr = reinterpret_cast<const char*>(&data);
        stream.write(ptr, sizeof(T));
    }
};

#endif