cmake_minimum_required(VERSION 3.10)
project(Syncer)

# Put all compiled executables neatly into build/bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 1. Gather all source files
file(GLOB SERVER_SOURCES "server/*.cpp")
file(GLOB CLIENT_SOURCES "client/*.cpp")

# 2. Create an INTERFACE library for the header-only shared code
# Since EnTT is in the shared folder, this automatically gives EnTT to your client/server!
add_library(shared_lib INTERFACE)
target_include_directories(shared_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/shared)

# 3. Define the executables
add_executable(syncer_server ${SERVER_SOURCES})
add_executable(syncer_client ${CLIENT_SOURCES})

# 4. Link everything together
if(WIN32)
    # Link shared_lib (which contains EnTT) and the Windows networking libraries
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")

    target_link_libraries(syncer_server PRIVATE shared_lib ws2_32 winmm)
    target_link_libraries(syncer_client PRIVATE shared_lib ws2_32 winmm)
else()
    # If on Linux/WSL, just link the shared headers
    target_link_libraries(syncer_server PRIVATE shared_lib)
    target_link_libraries(syncer_client PRIVATE shared_lib)
endif()