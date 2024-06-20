#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
#include "router.hpp"
#include "server.hpp"

using json = nlohmann::json;

int main() { 
    http::Router router;
    std::unique_ptr <http::TCPServer> server = std::make_unique <http::TCPServer> (8000, router); 

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
        json j = {
            {"email", "11@mail.ru"}, 
            {"password", "12345"}
        };
        router.POST(client_fd, j);
    });
    
    router.HandlerFunc("PUT", "/data/put/file", [&router](int client_fd){
        /*std::stringstream html;
        html << "<!DOCTYPE html>\n";
        html << "<html>\n";
        html << "<head>\n";
        html << "<meta charset='UTF-8'>\n";
        html << "<meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>\n";
        html << "<title>Web-Server</title>\n";
        html << "</head>\n";
        html << "<body>\n";
        html << "<h1>Welcome to My Website</h1>\n";
        html << "<h2></h2>\n";
        html << "<p><h3>PUT</h3></p>\n";
        html << "</body>\n";
        html << "</html>\n";*/
        std::ifstream file("index.html");
        router.PUT(client_fd, file);
    });

    router.HandlerFunc("PUT", "/data/put/stream", [&router](int client_fd){
        std::stringstream html;
        html << "<!DOCTYPE html>\n";
        html << "<html>\n";
        html << "<head>\n";
        html << "<meta charset='UTF-8'>\n";
        html << "<meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>\n";
        html << "<title>Web-Server</title>\n";
        html << "</head>\n";
        html << "<body>\n";
        html << "<h1>Welcome to My Website</h1>\n";
        html << "<h2></h2>\n";
        html << "<p><h3>PUT</h3></p>\n";
        html << "</body>\n";
        html << "</html>\n";
        //std::ifstream file("index.html");
        router.PUT(client_fd, html);
    });

    router.HandlerFunc("DELETE", "/data/delete", [&router](int client_fd){
        router.DELETE(client_fd);
    });

    if (server->Init()) {
        server->Start();
    }

    return 0;
}
