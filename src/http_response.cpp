#include "server/http_response.hpp"

#include <string>

namespace http_server {
namespace {

const char * reasonPhrase(int status) {
    switch (status) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        default: return "Unknown";
    }
}
} // namespace

std::string HttpResponse::serialize() const {
    std::string result = "HTTP/1.1 ";
    result += std::to_string(status);
    result+= " ";
    result+= reasonPhrase(status);
    result+= "\r\n";

    if (status != 204) {
        result+= "Content-Type: application/json\r\n";
        result+= "Content-Length: ";
        result+= std::to_string(body.size());
        result += "\r\n";
    }

    for (const auto& [name, value] : headers) {
        result += name + ": " + value + "\r\n";
    }
     result += "Connection: ";
     result += keep_alive ? "keep-alive\r\n" : "close\r\n";
     result += "\r\n";

     if (status != 204) {
        result += body;
     }

     return result;
}


HttpResponse handleRequest(const HttpRequest& request) {
    HttpResponse response;
    response.keep_alive = request.keep_alive;

    if (request.path != "/health") {
        response.status = 404;
        response.body = R"({"error":"Not Found"})";
    } else if (request.method != "GET") {
        response.status = 405;
        response.body = R"({"error":"method not allowed"})";
        response.headers.emplace_back("Allow", "GET");
    } else {
        response.status = 200;
        response.body = R"({"status":"ok"})";
    }
    
    return response;
}

} //namespace http_server