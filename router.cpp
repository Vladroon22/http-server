#include <netinet/in.h>
#include <string>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include "server.hpp"
#include "router.hpp"

using json = nlohmann::json;
using namespace http;

Router::Router() = default;

Router::~Router() = default;

void Router::Send(int clientSocket, const std::string& data) {
    if (send(clientSocket, data.c_str(), data.length(), 0) < 0) {
        ErrorResp(StatusCode::InternalServerError);
        std::cerr << "ERROR: sending data to client\n\n";
        close(clientSocket);
    }
}

std::string getErrorDescription(StatusCode error) {
    switch (error) {
        case StatusCode::NoContent:
            return "No Content";
        case StatusCode::BadRequest:
            return "Bad Request";
        case StatusCode::Unauthorized:
            return "Unauthorized";
        case StatusCode::Forbidden:
            return "Forbidden";
        case StatusCode::NotFound:
            return "Not Found";
        case StatusCode::MethodNotAllowed:
            return "Method Not Allowed";
        case StatusCode::InternalServerError:
            return "Internal Server Error";
        case StatusCode::NotImplemented:
            return "Not Implemented";
        default:
            return "Unknown Error";
    }
}

template<typename StatusCode>
std::string Router::ErrorResp(StatusCode error) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::gmtime(&now_c);  

    std::string desc_error = getErrorDescription(error);
    int error_code = static_cast<int>(error); 
    html << "<!DOCTYPE html>\n <html>\n <head> <meta charset='UTF-8'>\n <meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>\n";
    html << "<title>" << desc_error << "</title>\n";
    html << "</head>\n <body>" << std::to_string(error_code) + " " + desc_error << "</body>\n </html>\n";
    response << "HTTP/1.1 " + std::to_string(error_code) + " " + desc_error << "\r\n";
    response << "Date: " << std::put_time(&now_tm, "%a, %d %b %Y %H:%M:%S GMT") << "\r\n" << "Content-Type: text/html; charser=utf-8\r\n Content-Length: " << html.str().length() << "\r\nConnection: close\r\n";
    response << "\r\n\r\n";
    response << html.str();
    return response.str();
}


void Router::HandleFunc(const std::string& method, const std::string& path, std::function<void(int, Request)> handler) {
    for (auto it = route.begin(); it != route.end(); ++it) {
        if (it->path != path && it->method != method && it == route.end()) {
            Send(it->id, ErrorResp(StatusCode::BadRequest));
            close(it->id);
        } else if (it->path == path && it->method == method) {
            break;
        } else {
            continue;
        }
    }
}


void Router::HandleRequest(int fd, const std::string& method, const std::string& path) {
    struct Route r;
    r.method = std::move(method);
    r.path = std::move(path);
    r.id = fd;
    route.push_back(r);
}
