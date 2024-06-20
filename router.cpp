#include <fstream>
#include <netinet/in.h>
#include <string>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include "nlohmann/json.hpp"
#include "server.hpp"

using json = nlohmann::json;
using namespace http;

Router::Router() = default;

Router::~Router() = default;

void Router::Send(int clientSocket, const std::string &data) {
    if (send(clientSocket, data.c_str(), data.length(), 0) < 0) {
        ErrorResp(HTTPErrors::InternalServerError);
        std::cerr << "ERROR: sending data to client\n\n";
        close(clientSocket);
    }
}

std::string getErrorDescription(HTTPErrors error) {
    switch (error) {
        case HTTPErrors::BadRequest:
            return "Bad Request";
        case HTTPErrors::Unauthorized:
            return "Unauthorized";
        case HTTPErrors::Forbidden:
            return "Forbidden";
        case HTTPErrors::NotFound:
            return "Not Found";
        case HTTPErrors::MethodNotAllowed:
            return "Method Not Allowed";
        case HTTPErrors::InternalServerError:
            return "Internal Server Error";
        case HTTPErrors::NotImplemented:
            return "Not Implemented";
        default:
            return "Unknown Error";
    }
}

template<typename HTTPErrors>
std::string Router::ErrorResp(HTTPErrors error) {
    std::string desc_error = getErrorDescription(error);
    int error_code = static_cast<int>(error); 
    html << "<!DOCTYPE html>\n <html>\n <head> <meta charset='UTF-8'>\n <meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>\n";
    html << "<title>" << desc_error << "</title>\n";
    html << "</head>\n <body>" << std::to_string(error_code) + " " + desc_error << "</body>\n </html>\n";
    response << "HTTP/1.1" + std::to_string(error_code) + " " + desc_error << "\r\nContent-Type: text/html; charser=utf-8\r\n Content-Length: " << html.str().length() << "\r\nConnection: close\r\n";
    response << "\r\n\r\n";
    response << html.str();
    return response.str();
}

void Router::GET(int client_fd) {
    html << "<!DOCTYPE html>\n";
    html << "<html>\n";
    html << "<head>\n";
    html << "<meta charset='UTF-8'>\n";
    html << "<meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>\n";
    html << "<title>My-Web-Server-in-C++</title>\n";
    html << "</head>\n";
    html << "<body>\n";
    html << "<h1>Welcome to My Website</h1>\n";
    html << "<h2>My Web-Server in C++</h2>\n";
    html << "<p>This is the main content of my web-page.</p>\n";
    html << "</body>\n";
    html << "</html>\n";

    if (html.str().empty()) {
        Send(client_fd, ErrorResp(HTTPErrors::NotFound));
        close(client_fd);
        return;
    }
    
    response << "HTTP/1.1 200 OK\r\n";
    response << "Host: 127.0.0.1:8000\r\n";
    response << "Content-Type: text/html; charset=utf-8\r\n";
    response << "Content-Length: " << html.str().length() << "\r\n";
    response << "Connection: keep-alive\r\n\r\n";
    response << html.str();
    Send(client_fd, response.str());
}

void Router::POST(int client_fd, json& j) {
    if (j.dump().empty()) {
        Send(client_fd, ErrorResp(HTTPErrors::NotFound));
        close(client_fd);
        return;
    }

    j.push_back("key1=value1&key2=value2");  
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

    response << "HTTP/1.1 200 OK\r\n";
    response << oss.str() << "\r\n";
    response << "Connection: keep-alive\r\n\r\n";
    Send(client_fd, response.str());
}



void Router::HandleFile(int client_fd, std::ifstream& file){
    if (!file.is_open()) {
        ErrorResp(HTTPErrors::NotFound);
        std::cerr << "Failed to open file\n" << std::endl;  
        close(client_fd);
        return;          
    }
    std::string buffer;
    while (std::getline(file, buffer)) {
        html << buffer << std::endl;
    }  

    if (html.str().empty()) {
        response << "HTTP/1.1 204 No Content\r\n";
        response << "Content-Location: " << path << "\r\n";
        response << "Content-Type: text/html; charset=utf-8" << "\r\n";
        response << "Connection: keep-alive\r\n\r\n";
        response << html.str();
    } else {
        response << "HTTP/1.1 201 Created\r\n";
        response << "Content-Location: " << path << "\r\n";
        response << "Content-Type: text/html; charset=utf-8" << "\r\n";
        response << "Content-Length: " << html.str().size() << "\r\n";
        response << "Connection: keep-alive\r\n\r\n";
        response << html.str();
    }
    file.close();
}

void Router::JSON(const json &j){
    if (j.dump().empty()) {
        response << "HTTP/1.1 204 No Content\r\n";
        response << "Content-Location: " << path << "\r\n";
        response << "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n";
        response << "Connection: keep-alive\r\n\r\n";
        response << j.dump();
    } else {
        response << "HTTP/1.1 201 Created\r\n";
        response << "Content-Location: " << path << "\r\n";
        response << "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n";
        response << "Content-Length: " << j.dump().size() << "\r\n";
        response << "Connection: keep-alive\r\n\r\n";
        response << j.dump();
    }
}

void Router::HandleStream(int client_fd, std::stringstream& ss) {
    if (ss.str().empty()) {
        Send(client_fd, ErrorResp(HTTPErrors::NotFound));
        close(client_fd);
        return;
    }
    
    response << "HTTP/1.1 200 OK\r\n";
    response << "Host: 127.0.0.1:8000\r\n";
    response << "Content-Type: text/html; charset=utf-8\r\n";
    response << "Content-Length: " << html.str().length() << "\r\n";
    response << "Connection: keep-alive\r\n\r\n";
    response << html.str();
}

void Router::Set_header(const std::string &header) {
    response << header << "\r\n";
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
        Send(fd, ErrorResp(HTTPErrors::InternalServerError));
        close(fd);
    }
}
