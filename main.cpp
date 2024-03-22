#include <iostream>
#include <memory>
#include "server.hpp"


int main() { 
    std::unique_ptr <TCPServer> server = std::make_unique <TCPServer> (8000);
    try { 
        if (server->Init()) {
            server->Start();
        } 
    } catch(std::exception &ex) {
        std::cout << ex.what() << std::endl;
    } catch (...) {
        std::cout << "Execiption: Something goes wrong!" << std::endl;
    }

    return 0;
}
