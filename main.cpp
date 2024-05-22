#include <memory>
#include "router.hpp"
#include "server.hpp"


int main() { 
    std::unique_ptr <http::TCPServer> server = std::make_unique <http::TCPServer> (8000); 
    if (server->Init()) {
        server->Start();
    }
    
    http::Router router;
    
    router.HandlerFunc("GET", "/data", [&router](int client_fd){
        router.GET(client_fd);
    });

    router.HandlerFunc("POST", "/data/post", [&router](int client_fd){
        router.POST(client_fd);
    });

    router.HandlerFunc("PUT", "/data/put", [&router](int client_fd){
        router.POST(client_fd);
    });

    router.HandlerFunc("DELETE", "/data/delete", [&router](int client_fd){
        router.POST(client_fd);
    });


    return 0;
}
