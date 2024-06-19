#include <cstring>
#include <ctime>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include "server.hpp"
#include "router.hpp"


constexpr int MAX_EVENTS = 32;

using namespace http;

TCPServer::TCPServer(int port, Router& router) : port(port), serverSocket(-1), epollFD(-1), router(router) {}

TCPServer::~TCPServer() {
  if (serverSocket != -1 || epollFD != -1) {
    close(serverSocket);
    close(epollFD);
    std::cout << "The connection disabled!\n";
  } 
}

void TCPServer::NoBlock(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

bool TCPServer::Init() {
  serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (serverSocket == -1) {
    std::cerr << "ERROR: creating server socket\n";
    return false;
  }

  struct sockaddr_in server;
  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_port = htons(port);

  int st = 1;
  if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &st, sizeof(st)) < 0) {
    std::cerr << "ERROR: setsockopt\n";
    return false;
  }

  if (bind(serverSocket, (struct sockaddr *)&server, sizeof(server)) == -1) {
    std::cerr << "ERROR: binding server socket\n";
    return false;
  }

  if (listen(serverSocket, 15) == -1) {
    std::cerr << "ERROR: listening on server socket\n";
    return false;
  }

  NoBlock(serverSocket);

  epollFD = epoll_create1(0);
  if (epollFD == -1) {
    std::cerr << "ERROR: creating epoll instance\n";
    return false;
  }

  AddToEpoll(serverSocket);

  std::cout << "Server is listening" << std::endl;
  std::cout << "Server: " << "http://" << inet_ntoa(server.sin_addr) << ":" << ntohs(server.sin_port) << "\n" << std::endl;

  return true;
}

void TCPServer::Send(int clientSocket, const std::string &data) {
  if (send(clientSocket, data.c_str(), data.length(), O_NONBLOCK) < 0) {
    router.ErrorResp(HTTPErrors::InternalServerError);
    std::cerr << "ERROR: sending data to client\n\n";
    close(clientSocket);
  }
}

std::vector<char> TCPServer::Received(int client_fd) {
  std::vector<char> buffer(2048);
  int rcv = recv(client_fd, buffer.data(), buffer.size(), O_NONBLOCK);

  if (rcv < 0) {
    router.ErrorResp(HTTPErrors::InternalServerError);
    std::cout << "Client disconnected\n";
    close(client_fd);
    buffer.clear();
    return buffer;
  } 

  return buffer;
}

void TCPServer::Start() {
  while (serverSocket > 0) {
    epoll_event events[MAX_EVENTS];
    int numEvents = epoll_wait(epollFD, events, MAX_EVENTS, -1);
    for (int i = 0; i < numEvents; ++i) {
      if (events[i].data.fd == serverSocket) {
        AcceptConnection(events[i].data.fd);
      } else {
        Response(events[i].data.fd);
      }
    }
  }
}




void TCPServer::Response(int clientSocket) {
  std::vector<char> buffer = Received(clientSocket);
  if (buffer.empty()) {
    Send(clientSocket, router.ErrorResp(HTTPErrors::InternalServerError));
    close(clientSocket);
    return;
  }
  std::string request(buffer.begin(), buffer.end());
  std::istringstream requestStream(request);
  std::string method, path;
  requestStream >> method >> path;

  router.HandleRequest(clientSocket, method, path);
  std::cout << "Received: " << buffer.data() << std::endl;  
  close(clientSocket);
}

void TCPServer::AddToEpoll(int fd) {
  epoll_event event{};
  event.data.fd = fd;
  event.events = EPOLLIN | EPOLLET;
  epoll_ctl(epollFD, EPOLL_CTL_ADD, fd, &event);
}

void TCPServer::AcceptConnection(int client_fd) {
  struct sockaddr_in client;
  socklen_t clientLen = sizeof(client);

  client_fd = accept(serverSocket, (struct sockaddr *)&client, &clientLen);
  if (client_fd < 0) {
    router.ErrorResp(HTTPErrors::BadRequest);
    std::cerr << "ERROR: not accepted connection\n";
    close(client_fd);
    return;
  }

  NoBlock(client_fd);
  AddToEpoll(client_fd);

  std::cout << "New client was accepted: " << inet_ntoa(client.sin_addr) << ":" << ntohs(client.sin_port) << "\n";
}
