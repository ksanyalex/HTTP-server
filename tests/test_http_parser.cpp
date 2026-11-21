
#include "server/http_parser.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

using namespace server;

// Проверяем условие. Если оно ложное — тест не пройден.
void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

// Тест 1. Обычный GET-запрос
void testGet() {
    HttpParser parser;

    parser.append(
        "GET /health HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n"
    );

    auto result = parser.parseNext();

    check(result.status == ParseStatus::Complete,
          "GET should be complete");
    check(result.request.has_value(),
          "GET request should exist");
    check(result.request->method == "GET",
          "Wrong GET method");
    check(result.request->path == "/health",
          "Wrong GET path");
    check(result.request->version == "HTTP/1.1",
          "Wrong HTTP version");
}

// Тест 2. POST с телом
void testPost() {
    HttpParser parser;

    const std::string body = R"({"name":"Pencil"})";

    parser.append(
        "POST /items HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: " +
        std::to_string(body.size()) +
        "\r\n\r\n" + body
    );

    auto result = parser.parseNext();

    check(result.status == ParseStatus::Complete,
          "POST should be complete");
    check(result.request.has_value(),
          "POST request should exist");
    check(result.request->method == "POST",
          "Wrong POST method");
    check(result.request->path == "/items",
          "Wrong POST path");
    check(result.request->body == body,
          "Wrong POST body");
}

// Тест 3. Запрос приходит частями
void testIncomplete() {
    HttpParser parser;

    parser.append(
        "POST /items HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "He"
    );

    auto first = parser.parseNext();

    check(first.status == ParseStatus::Incomplete,
          "Request should be incomplete");

    parser.append("llo");

    auto second = parser.parseNext();

    check(second.status == ParseStatus::Complete,
          "Request should be complete");
    check(second.request.has_value(),
          "Completed request should exist");
    check(second.request->body == "Hello",
          "Wrong assembled body");
}

// Тест 4. Несколько запросов через keep-alive
void testKeepAlive() {
    HttpParser parser;

    parser.append(
        "GET /health HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n"
        "GET /items HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Connection: close\r\n"
        "\r\n"
    );

    auto first = parser.parseNext();

    check(first.status == ParseStatus::Complete,
          "First request failed");
    check(first.request.has_value(),
          "First request missing");
    check(first.request->path == "/health",
          "Wrong first path");
    check(first.request->keep_alive,
          "Keep-alive should be enabled");

    auto second = parser.parseNext();

    check(second.status == ParseStatus::Complete,
          "Second request failed");
    check(second.request.has_value(),
          "Second request missing");
    check(second.request->path == "/items",
          "Wrong second path");
    check(!second.request->keep_alive,
          "Connection should close");
}

// Тест 5. Некорректные HTTP-запросы
void testInvalidRequest() {
    const std::string_view invalidRequests[] = {
        // Отсутствует Host
        "GET /health HTTP/1.1\r\n\r\n",

        // Некорректный Content-Length
        "POST /items HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Length: abc\r\n\r\n",

        // Неподдерживаемая версия HTTP
        "GET /health HTTP/1.0\r\n"
        "Host: localhost\r\n\r\n",

        // Превышен лимит тела
        "POST /items HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Length: 1048577\r\n\r\n"
    };

    for (auto request : invalidRequests) {
        HttpParser parser;

        parser.append(request);

        auto result = parser.parseNext();

        check(result.status == ParseStatus::Error,
              "Invalid request was accepted");
    }
}

int main() {
    testGet();
    testPost();
    testIncomplete();
    testKeepAlive();
    testInvalidRequest();

    std::cout << "All 5 tests passed!\n";
    return 0;
}
