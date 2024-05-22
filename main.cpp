#include <memory>
#include "router.hpp"
#include "server.hpp"


int main() { 
    std::unique_ptr <http::TCPServer> server = std::make_unique <http::TCPServer> (8000); 
    
    http::Router &router = server->GetRouter();
    
    router.HandlerFunc("GET", "/", [&router](int client_fd){
        router.GET(client_fd);
    });

    router.HandlerFunc("GET", "/data", [&router](int client_fd){
        router.GET(client_fd);
    });

    router.HandlerFunc("GET", "/data/get", [&router](int client_fd){
        router.GET(client_fd);
    });

    router.HandlerFunc("POST", "/data/post", [&router](int client_fd){
        router.POST(client_fd);
    });

    router.HandlerFunc("PUT", "/data/put", [&router](int client_fd){
        router.PUT(client_fd);
    });

    router.HandlerFunc("DELETE", "/data/delete", [&router](int client_fd){
        router.DELETE(client_fd);
    });

    if (server->Init()) {
        server->Start();
    }

    return 0;
}
