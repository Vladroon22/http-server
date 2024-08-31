#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "router.hpp"
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;
namespace http {

class Request { 
    public:
        Request();
        ~Request();
        
        void Set_header(const std::string& new_header);
        void Del_header(const std::string& which_header);
        void JSON(int client_fd, const json& j);
        void HandleFile(int client_fd, const std::string& filename);
        void HandleStream(int client_fd, std::stringstream& ss);            
    
        template<typename HTTPErrors>
        std::string ErrorResp(HTTPErrors error);

        void GET(int client_fd, std::string& filename);
        void DELETE(int client_fd);
    
        template<typename T>
        void POST(int client_fd, const T& j);
        
        template<typename T>
        void PUT(int client_fd, const T& j);

    private:
        friend class Router; 
        void Send(int clientSocket, const std::string& data);
    private:
        std::stringstream html;
        std::stringstream response;
        json j;
        std::string method;
        std::string path; 
};

} // http

#endif