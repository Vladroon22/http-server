#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <functional>
#include <string>
#include <unordered_map>

namespace http {

class Router { 
    public:
        Router();
        ~Router();
        void GET(int client_fd);
        void POST(int client_fd);
        void PUT(int client_fd);
        void DELETE(int client_fd);
        void HandleFunc(int fd, const std::string &method, const std::string &path, std::function<void(int)> rout);
        void HandleRequest(int id, const std::string &method, const std::string &path);
    private:
        void Send(int clientSocket, const std::string &data);
        std::string genPath(const std::string &method, const std::string &path);
    private:
        std::string method;
        std::string path;
        std::unordered_map<std::string, std::function<void(int)>> routes;
};

} // http

#endif