#include <memory>
#include <nlohmann/json.hpp>
#include "request.hpp"
#include "router.hpp"
#include "server.hpp"

using json = nlohmann::json;


int main() { 
    http::Router router;

    router.HandleFunc("GET", "/", [&router](int client_fd, http::Request &r){
       r.GET(client_fd, "index.html");
    });

    std::unique_ptr <http::TCPServer> server = std::make_unique <http::TCPServer> ("127.0.0.1", 8000, router); 

    if (server->Init()) {
        server->Start();
    }

    return 0;
}
