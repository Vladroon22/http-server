#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <nlohmann/json.hpp>
#include <functional>
#include <sstream>
#include <string>
#include <unordered_map>

using json = nlohmann::json;
namespace http {

class Router { 
    public:
        Router();
        ~Router();
        void GET(int client_fd);
        void POST(int client_fd, json& j);
        void DELETE(int client_fd);
        void HandlerFunc(const std::string &method, const std::string &path, std::function<void(int)> handler);
        void Set_header(const std::string& new_header);
        void JSON(const json& j);
        void HandleFile(int client_fd, const std::string& filename);
        void HandleStream(int client_fd, std::stringstream& ss);        

        template<typename HTTPErrors>
        std::string ErrorResp(HTTPErrors error);

        template<typename T>
        void PUT(int client_fd, void(&func)(int,T&)) { 
            T data;
            func(client_fd, data);
            Send(client_fd, response.str());
        }
    private:
        friend class TCPServer;
        void HandleRequest(int id, const std::string &method, const std::string &path);
        void Send(int clientSocket, const std::string &data);
        std::string genPath(const std::string &method, const std::string &path);
    private:
        std::stringstream html;
        std::stringstream response;
        json j;
        std::string method;
        std::string path;
        std::unordered_map<std::string, std::function<void(int)>> routes;
};

} // http

#endif