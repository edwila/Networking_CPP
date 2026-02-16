#include "packets.h"
#include <winsock2.h>
#include <vector>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>

constexpr static const char* PORT = "8080";

constexpr static int MAX_PLAYERS = 10; // How many connections we'll allow

// Create a class that'll handle all of the server's communication
class network {
private:
    using bufferType = std::array<char, BUFFER_SIZE>;
    int socket_descriptor;
    addrinfo hints, *res;
    std::vector<bufferType> buffers; // Index will be the player who's sending requests
    std::thread receiver;
    std::vector<sockaddr*> connected_clients;
public:
    network(): socket_descriptor(socket(AF_INET, SOCK_DGRAM, 0)) {
        assert(socket_descriptor != 0); // Ensure socket was initialized

        memset(&hints, 0, sizeof(hints)); // Will zero out the padding
        
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE;

        getaddrinfo(NULL, PORT, &hints, &res);

        buffers.resize(MAX_PLAYERS); // TODO: change to reserve and insert/remove as needed

        bind(socket_descriptor, res->ai_addr, res->ai_addrlen);
    }

    void process(){
        // Listen, then accept() and populate buffer

        this->receiver = std::thread([this](){
            sockaddr_storage user_info;

            socklen_t sz = sizeof(user_info);

            int rec = recvfrom(socket_descriptor, buffers[0].data(), BUFFER_SIZE-1, 0, (struct sockaddr *)&user_info, &sz);
        });
    }

    void send_to_all(const std::vector<uint8_t>& data_buf){
        // Send data_buf to all connected clients

        //for(auto client : connected_clients){
          //  sendto(socket_descriptor, (const char*)data_buf.data(), );
        //}
    }

    void clean_up(){
        closesocket(socket_descriptor);

        if(this->receiver.joinable()){
            this->receiver.join();
        }

        std::cout << "Network process joined successfully.\n";
    }

    ~network(){
        this->clean_up();
    }
};