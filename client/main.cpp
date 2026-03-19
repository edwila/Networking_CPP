#include "./syncer.h"
#include <iostream>


int main(){
    std::cout << "CLIENT!\n";

    Syncer syncer(nullptr);

    syncer.init();

    std::string user_entry;

    while(user_entry != "1"){
        std::cout << "Option: ";
        std::cin >> user_entry;
        if(user_entry == "disconnect"){
            syncer.disconnect();
        } else if(user_entry == "reconnect"){
            syncer.init(); // Typically should be something like "syncer.connect()" but since the init() only calls net.process(), this is fine
        }
    }
    
    return 0;
}