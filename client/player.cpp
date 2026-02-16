#include "player.h"
#include "syncer.h"
#include <iostream>


int main(){
    std::cout << "CLIENT!\n";

    Syncer syncer(nullptr);

    syncer.init();

    std::string user_entry;

    while(user_entry != "1"){
        std::cout << "Option: ";
        std::cin >> user_entry;
    }
    
    return 0;
}