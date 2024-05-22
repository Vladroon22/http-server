#include <netinet/in.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sstream>
#include <iostream>
#include "router.hpp"

using namespace http;

Router::Router() = default;

Router::~Router() = default;

void Router::Send(int clientSocket, const std::string &data) {
    if (send(clientSocket, data.c_str(), data.length(), O_NONBLOCK) < 0) {
        std::cerr << "ERROR: sending data to client\n\n";
        close(clientSocket);
    }
}

void Router::GET(int client_fd) {
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
    response << "Content-Type: text/html; charser=utf-8\r\n";
    response << "Content-Length: " << htmlContent.str().length() << "\r\n";
    response << "Connection: keep-alive\r\n\r\n";
    response << htmlContent.str();
    Send(client_fd, response.str());
}

void Router::POST(int client_fd) {

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
    response << "Content-Type: text/html; charser=utf-8\r\n";
    response << "Content-Length: " << htmlContent.str().length() << "\r\n";
    response << "Connection: keep-alive\r\n\r\n";
    response << htmlContent.str();
    Send(client_fd, response.str());
}

void Router::DELETE(int client_fd) {

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
    response << "Content-Type: text/html; charser=utf-8\r\n";
    response << "Content-Length: " << htmlContent.str().length() << "\r\n";
    response << "Connection: keep-alive\r\n\r\n";
    response << htmlContent.str();
    Send(client_fd, response.str());
}

void Router::PUT(int client_fd) {

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
    response << "Content-Type: text/html; charser=utf-8\r\n";
    response << "Content-Length: " << htmlContent.str().length() << "\r\n";
    response << "Connection: keep-alive\r\n\r\n";
    response << htmlContent.str();
    Send(client_fd, response.str());
}

std::string Router::genPath(const std::string &method, const std::string &path) {
    return method + ":" + path;
}

void Router::HandlerFunc(const std::string &method, const std::string &path, std::function<void(int)> handler){
    std::string key = genPath(method, path);
    routes[key] = handler;
}

void Router::HandleRequest(int fd, const std::string &method, const std::string &path) {
    std::string key = genPath(method, path);
    if (routes.find(key) != routes.end()) {
        routes[key](fd);
    } else {
        std::stringstream errorResponse;
        errorResponse << "HTTP/1.1 404 Not Found\r\nContent-Type: text/html; charser=utf-8\r\n Content-Length: 0\r\nConnection: close\r\nerrorResponse << \r\n\r\n";
        errorResponse << "<!DOCTYPE html>\n <html>\n <head> <meta charset='UTF-8'>\n <meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>\n <title>PAGE NOT FOUND</title>\n </head>\n <body> PAGE NOT FOUND </body>\n";
        Send(fd, errorResponse.str());
        close(fd);
    }
}

