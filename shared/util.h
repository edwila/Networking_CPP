#ifndef UTIL_LIB_SYNCER_H
#define UTIL_LIB_SYNCER_H
#include <iostream>
#include <utility>

template<typename... Args>
void out(Args&&... args){
    std::cout << "\r\033[2K";
    ((std::cout << std::forward<Args>(args)), ...);
    std::cout << "\n$ " << std::flush;
}
#endif