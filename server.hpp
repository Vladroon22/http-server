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

namespace http{

class TCPServer {
    public:
        TCPServer(int port);
        ~TCPServer();
        bool Init();
        void Start();
        
    private:
        void NoBlock(int fd);
        void Send(int clientSocket, const std::string& data);
        std::vector <char> Received(int client_fd);
        void Response(int clientSocket);
        void AddToEpoll(int fd);
        void AcceptConnection(int client_fd);

    private:
        int port;
        int serverSocket;
        int epollFD;
};

}; 
#endif