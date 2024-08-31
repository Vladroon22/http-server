#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "request.hpp"
#include <nlohmann/json.hpp>
#include <functional>
#include <string>

using json = nlohmann::json;
namespace http {

class Router { 
    public:
        Router();
        ~Router();        
        
        void HandleFunc(const std::string& method, const std::string& path, std::function<void(int, Request)> handler);

        template<typename HTTPErrors>
        std::string ErrorResp(HTTPErrors error);    

    private:
        friend class TCPServer;
        void HandleRequest(int id, const std::string& method, const std::string& path);
        void Send(int clientSocket, const std::string& data);
    private:
        struct Route {
            std::string method;
            std::string path;
            int id;
        }; 
    private:
        std::stringstream html;
        std::stringstream response;
        std::vector <Route> route;
};



} // http

#endif