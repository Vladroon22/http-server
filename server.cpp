#include <cstring>
#include <ctime>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "server.hpp"


constexpr int MAX_EVENTS = 32;

TCPServer::TCPServer(int port) : port(port), serverSocket(-1), epollFD(-1) {}

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
    std::cerr << "ERROR: sending data to client\n\n";
    close(clientSocket);
  }
}

std::vector<char> TCPServer::Received(int client_fd) {
  std::vector<char> buffer(2048);
  int rcv = recv(client_fd, buffer.data(), buffer.size(), O_NONBLOCK);

  if (rcv < 0) {
    std::cout << "Client disconnected\n";
    close(client_fd);
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
    std::stringstream errorResponse;
    errorResponse << buffer.data() << "\r\n";
    errorResponse << "Content-Type: text/html; charser=utf-8\r\n";
    errorResponse << "Content-Length: 0\r\n";
    errorResponse << "Connection: close\r\n";
    errorResponse << "\r\n\r\n";
    
    Send(clientSocket, errorResponse.str());

  } else {
    std::cout << "Received: " << buffer.data() << std::endl;

    std::time_t currentTime = std::time(nullptr);
    std::tm *localTime = std::localtime(&currentTime);
    char currDate[64];
    std::strftime(currDate, sizeof(currDate), "%a, %d %b %Y %H:%M:%S GMT",localTime);

    std::stringstream htmlContent;
    htmlContent << "<!DOCTYPE html>\n";
    htmlContent << "<html>\n";
    htmlContent << "<head>\n";
    htmlContent << "<meta charset='UTF-8'>\n";
    htmlContent << "<meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>\n";
    htmlContent << "<title>My-Web-Server-in-C++</title>\n";
    htmlContent << "</head>\n";
    htmlContent << "<body>\n";
    htmlContent << "<h1>Welcome to My Website</h1>\n";
    htmlContent << "<h2>My Web-Server in C++</h2>\n";
    htmlContent << "<p>This is the main content of my web-page.</p>\n";
    htmlContent << "</body>\n";
    htmlContent << "</html>\n";

    std::stringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Version: HTTP/1.1 \r\n";
    response << "Server: MyServerC++\r\n";
    response << "Content-Type: text/html; charset=utf-8\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Content-Length: " << htmlContent.str().length() << "\r\n";
    response << "Date: " << currDate << "\r\n";
    response << "Connection: keep-alive\r\n";
    response << "\r\n\r\n";
    response << htmlContent.str();

    Send(clientSocket, response.str());
  }
  
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
    std::cerr << "ERROR: not accepted connection\n";
    exit(EXIT_FAILURE);
  }

  AddToEpoll(client_fd);
  NoBlock(client_fd);

  std::cout << "New client was accepted: " << inet_ntoa(client.sin_addr) << ":" << ntohs(client.sin_port) << "\n";
}
