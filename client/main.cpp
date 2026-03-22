#include "./syncer.h"
#include <iostream>


int main(){
    std::cout << "CLIENT!\n";

    Syncer syncer(nullptr);

    syncer.init();

    std::string user_entry;

    while(user_entry != "exit"){
        std::cout << ">> ";
        std::cin >> user_entry;
        if(user_entry == "disconnect"){
            syncer.disconnect();
        } else if(user_entry == "connect"){
            std::cin >> user_entry;
            user_entry = user_entry == "local" ? "127.0.0.1" : user_entry;
            syncer.connect(user_entry);
        }
    }

    syncer.clean_up();
    
    return 0;
}