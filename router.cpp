#include <netinet/in.h>
#include <string>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include "router.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using namespace http;

Router::Router() = default;

Router::~Router() = default;

void Router::Send(int clientSocket, const std::string &data) {
    if (send(clientSocket, data.c_str(), data.length(), 0) < 0) {
        std::cerr << "ERROR: sending data to client\n\n";
        close(clientSocket);
    }
}


std::string Router::ErrorResp(){
  std::stringstream errorResponse, html;
  html << "<!DOCTYPE html>\n <html>\n <head> <meta charset='UTF-8'>\n <meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>\n <title>PAGE NOT FOUND</title>\n </head>\n <body> PAGE NOT FOUND </body>\n </html>\n";
  errorResponse << "HTTP/1.1 404 Not Found\r\nContent-Type: text/html; charser=utf-8\r\n Content-Length: " << html.str().length() << "\r\nConnection: close\r\nerrorResponse << \r\n\r\n";
  errorResponse << html.str();
  return errorResponse.str();
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

    if (htmlContent.str().empty()) {
        std::string err = ErrorResp();
        Send(client_fd, err);
        close(client_fd);
        return;
    }

    std::stringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Host: 127.0.0.1:8000\r\n";
    response << "Content-Type: text/html; charset=utf-8\r\n";
    response << "Content-Length: " << htmlContent.str().length() << "\r\n";
    response << "Connection: keep-alive\r\n\r\n";
    response << htmlContent.str();
    Send(client_fd, response.str());
}

void Router::POST(int client_fd) {
    json j = {
        {"foo", 1},
        {"bar", true},
        {"baz", 1.78}
    };


    if (j.dump().empty()) {
        std::string err = ErrorResp();
        Send(client_fd, err);
        close(client_fd);
        return;
    }

    std::stringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Host: 127.0.0.1:8000\r\n";
    response << "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n";
    response << "Content-Length: " << j.dump().size() << "\r\n";
    response << "Connection: keep-alive\r\n\r\n";
    response << j.dump();
    Send(client_fd, response.str());
}

void Router::DELETE(int client_fd) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::gmtime(&now_c);  
    std::ostringstream oss;
    oss << "Date: " << std::put_time(&now_tm, "%a, %d %b %Y %H:%M:%S GMT");

    std::stringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << oss.str() << "\r\n";
    response << "Connection: keep-alive\r\n\r\n";
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
    htmlContent << "<p><h3>PUT</h3></p>\n";
    htmlContent << "</body>\n";
    htmlContent << "</html>\n";

    if (htmlContent.str().empty()) {
        std::stringstream response;
        response << "HTTP/1.1 204 No Content\r\n";
        response << "Content-Location: " << path << "\r\n";
        response << "Connection: keep-alive\r\n\r\n";
        Send(client_fd, response.str());
    } else {
        std::stringstream response;
        response << "HTTP/1.1 201 Created\r\n";
        response << "Content-Location: " << path << "\r\n";
        response << "Content-Length: " << htmlContent.str().length() << "\r\n";
        response << "Connection: keep-alive\r\n\r\n";
        response << htmlContent.str();
        Send(client_fd, response.str());
    }
}

std::string Router::genPath(const std::string &method, const std::string &path) {
    return method + ":" + path;
}

void Router::HandlerFunc(const std::string &method, const std::string &path, std::function<void(int)> handler) {
    std::string key = genPath(method, path);
    routes[key] = handler;
}

void Router::HandleRequest(int fd, const std::string &method, const std::string &path) {
    std::string key = genPath(method, path);
    if (routes.find(key) != routes.end()) {
        routes[key](fd);
    } else {
        std::string err = ErrorResp();
        Send(fd, err);
        close(fd);
    }
}
