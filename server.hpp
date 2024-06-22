#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <vector>
#include "router.hpp"

namespace http{

class TCPServer {
    public:
        TCPServer(std::string, int port, Router& router);
        ~TCPServer();
        bool Init();
        void Start();

    private:
        void AcceptConnection(int client_fd);
        void NoBlock(int fd);
        void Send(int clientSocket, const std::string& data);
        std::vector <char> Received(int client_fd);
        void Response(int clientSocket);
        void AddToEpoll(int fd);

    private:
        int port;
        std::string address;
        int serverSocket;
        int epollFD;
        Router &router;
};

enum class StatusCode : int {
    NoContent = 204,
    BadRequest = 400, 
    Unauthorized = 401,
    Forbidden = 403, 
    NotFound = 404,
    MethodNotAllowed = 405,
    InternalServerError = 500, 
    NotImplemented = 501, 
};

}; 
#endif