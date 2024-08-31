#include <fstream>
#include <netinet/in.h>
#include <string>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include "nlohmann/json.hpp"
#include "router.hpp"
#include "server.hpp"
#include <regex>
#include "request.hpp"

using json = nlohmann::json;
using namespace http;

Request::Request() = default;

Request::~Request() = default;

void Request::Send(int clientSocket, const std::string& data) {
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
std::string Request::ErrorResp(StatusCode error) {
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



void Request::GET(int client_fd, std::string& filename) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::gmtime(&now_c);  
    
    if (html.str().empty()) {
        Send(client_fd, ErrorResp(StatusCode::NotFound));
        close(client_fd);
        return;
    }
    
    response << "HTTP/1.1 200 OK\r\n";
    response << "Date: " << std::put_time(&now_tm, "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
    response << "Content-Type: text/html; charset=utf-8\r\n";
    response << "Content-Length: " << html.str().length() << "\r\n";
    response << "Connection: keep-alive\r\n\r\n";
    response << html.str();
    Send(client_fd, response.str());
}

template<typename T>
void Request::POST(int client_fd, const T& j) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::gmtime(&now_c);  
    
    if (j.dump().empty()) {
        Send(client_fd, ErrorResp(StatusCode::NotFound));
        close(client_fd);
        return;
    }

    response << "HTTP/1.1 200 OK\r\n";
    response << "Date: " << std::put_time(&now_tm, "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
    response << "Content-Type: application/json; charset=utf-8\r\n";
    response << "Content-Length: " << j.dump().size() << "\r\n";
    response << "Connection: keep-alive\r\n\r\n";
    response << j.dump();
    Send(client_fd, response.str());
}

void Request::DELETE(int client_fd) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::gmtime(&now_c);  

    response << "HTTP/1.1 200 OK\r\n";
    response << "Date: " << std::put_time(&now_tm, "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
    response << "Connection: keep-alive\r\n\r\n";
    Send(client_fd, response.str());
}

template<typename T>
void Request::PUT(int client_fd, const T& j) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::gmtime(&now_c);  

    response << "HTTP/1.1 200 OK\r\n";
    response << "Date: " << std::put_time(&now_tm, "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
    response << "Content-Length: " << j.dump().size() << "\r\n";
    response << "Connection: keep-alive\r\n\r\n";
    response << j.dump();
}

void Request::HandleFile(int client_fd, const std::string& filename){
    std::ifstream file(filename);
    if (!file.is_open()) {
        ErrorResp(StatusCode::NotFound);
        std::cerr << "Failed to open file\n" << std::endl;  
        close(client_fd);
        return;          
    }
    std::string buffer;
    while (std::getline(file, buffer)) {
        html << buffer << std::endl;
    }  

    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::gmtime(&now_c);  

    if (html.str().empty()) {
        ErrorResp(StatusCode::NoContent);
    } else {
        response << "HTTP/1.1 201 Created\r\n";
        response << "Date: " << std::put_time(&now_tm, "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
        response << "Content-Location: " << path << "\r\n";
        response << "Content-Length: " << j.dump().size() << "\r\n";
        response << "Connection: keep-alive\r\n\r\n";
        response << j.dump();
    }
    file.close();
    Send(client_fd, response.str());
}

void Request::JSON(int client_fd, const json& j){
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::gmtime(&now_c);  

    if (j.dump().empty()) {
        ErrorResp(StatusCode::NoContent);
    } else {
        response << "HTTP/1.1 201 Created\r\n";
        response << "Date: " << std::put_time(&now_tm, "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
        response << "Content-Location: " << path << "\r\n";
        response << "Content-Length: " << j.dump().size() << "\r\n";
        response << "Connection: keep-alive\r\n\r\n";
        response << j.dump();
    }
    Send(client_fd, response.str());
}

void Request::HandleStream(int client_fd, std::stringstream& ss) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::gmtime(&now_c);  
    
    if (ss.str().empty()) {
        Send(client_fd, ErrorResp(StatusCode::NotFound));
        close(client_fd);
        return;
    }
    
    response << "HTTP/1.1 200 OK\r\n";
    response << "Date: " << std::put_time(&now_tm, "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
    response << "Content-Length: " << html.str().length() << "\r\n";
    response << "Connection: keep-alive\r\n\r\n";
    response << html.str();
    Send(client_fd, response.str());
}

void Request::Set_header(const std::string& header) {
    response << header << "\r\n";
}

void Request::Del_header(const std::string& which_header) {
    std::string content = response.str();
    std::regex pattern(which_header);
    content = std::regex_replace(content, pattern, "");
    response.str(std::string());
    response.clear();
    response << content;
}
