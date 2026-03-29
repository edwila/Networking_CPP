cmake_minimum_required(VERSION 3.10)
project(Syncer)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB SERVER_SOURCES "server/*.cpp")
file(GLOB CLIENT_SOURCES "client/*.cpp")

add_library(shared_lib INTERFACE)
target_include_directories(shared_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/shared)

add_executable(server ${SERVER_SOURCES})
add_executable(client ${CLIENT_SOURCES})

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")

    target_link_libraries(server PRIVATE shared_lib ws2_32 winmm)
    target_link_libraries(client PRIVATE shared_lib ws2_32 winmm)
else()
    target_link_libraries(server PRIVATE shared_lib)
    target_link_libraries(client PRIVATE shared_lib)
endif()