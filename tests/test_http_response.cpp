
#include "server/http_response.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

using namespace http_server;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

void testHealth() {
    HttpRequest request;
    request.method = "GET";
    request.path = "/health";

    const auto response = handleRequest(request);

    const std::string expected =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 15\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"
        "{\"status\":\"ok\"}";

    check(response.status == 200, "Wrong status for /health");
    check(response.serialize() == expected, "Wrong /health response");
}

void testConnectionClose() {
    HttpRequest request;
    request.method = "GET";
    request.path = "/health";
    request.keep_alive = false;

    const auto response = handleRequest(request);

    check(!response.keep_alive, "Connection must close");
    check(response.serialize().find("Connection: close\r\n") !=
              std::string::npos, "Missing Connection: close");
}

void testUnknownPathAndMethod() {
    HttpRequest request;
    request.method = "GET";
    request.path = "/missing";

    check(handleRequest(request).status == 404,
          "Unknown path must return 404");

    request.method = "POST";
    request.path = "/health";

    const auto response = handleRequest(request);

    check(response.status == 405,
          "Wrong method must return 405");
    check(response.serialize().find("Allow: GET\r\n") !=
              std::string::npos, "Missing Allow header");
}

void testNoContent() {
    HttpResponse response;
    response.status = 204;
    response.body = "should not be sent";

    const std::string expected =
        "HTTP/1.1 204 No Content\r\n"
        "Connection: keep-alive\r\n"
        "\r\n";

    check(response.serialize() == expected,
          "204 must not have body");
}

int main() {
    testHealth();
    testConnectionClose();
    testUnknownPathAndMethod();
    testNoContent();

    std::cout << "All 4 HTTP response tests passed!\n";
}
