#include <netinet/in.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include "router.hpp"

using namespace http;

void Router::Send(int clientSocket, const std::string &data) {
    if (send(clientSocket, data.c_str(), data.length(), O_NONBLOCK) < 0) {
        std::cerr << "ERROR: sending data to client\n\n";
        close(clientSocket);
    }
}

void Router::GET(int client_fd) {
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nGET request handled \n";
    Send(client_fd, response);
}

void Router::POST(int client_fd) {
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nPOST request handled \n";
    Send(client_fd, response);
}

void Router::DELETE(int client_fd) {
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nDELETE request handled \n";
    Send(client_fd, response);
}

void Router::PUT(int client_fd) {
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nPUT request handled \n";
    Send(client_fd, response);
}

std::string Router::genPath(const std::string &method, const std::string &path) {
    return method + ":" + path;
}

void Router::HandleFunc(int fd, const std::string &method, const std::string &path, std::function<void(int)> newrout){
    std::string key = genPath(path, method);
    routes[key] = newrout;
}

void Router::HandleRequest(int fd, const std::string &method, const std::string &path) {
    std::string errorResponse = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html; charser=utf-8\r\n Content-Length: 0\r\nConnection: close\r\nerrorResponse << \r\n\r\n";
    std::string key = genPath(method, path);
    if (routes.find(key) != routes.end()) {
        routes[key](fd);
    } else {
        Send(fd, errorResponse);
    }
}