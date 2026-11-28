
#include "server/http_parser.hpp"

#include <charconv>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

namespace http_server {
namespace {

bool isTokenChar(char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') ||
           std::string_view("!#$%&'*+-.^_`|~").find(c) !=
               std::string_view::npos;
}

std::string lowercase(std::string_view value) {
    std::string result(value);

    for (char& c : result) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }

    return result;
}

std::string_view trimOws(std::string_view text) {
    while (!text.empty() &&
           (text.front() == ' ' || text.front() == '\t')) {
        text.remove_prefix(1);
    }

    while (!text.empty() &&
           (text.back() == ' ' || text.back() == '\t')) {
        text.remove_suffix(1);
    }

    return text;
}

bool hasConnectionToken(std::string_view value,
                        std::string_view token) {
    while (!value.empty()) {
        const auto comma = value.find(',');
        const auto part = trimOws(value.substr(0, comma));

        if (lowercase(part) == token) {
            return true;
        }

        if (comma == std::string_view::npos) {
            break;
        }

        value.remove_prefix(comma + 1);
    }

    return false;
}

bool validTarget(std::string_view target) {
    if (target.empty() || target.front() != '/') {
        return false;
    }

    for (unsigned char c : target) {
        if (c <= 0x20 || c >= 0x7f || c == '#') {
            return false;
        }
    }

    return true;
}

}  // namespace

void HttpParser::append(std::string_view bytes) {
    if (bytes.empty() || !error_.empty()) {
        return;
    }

    if (bytes.size() > kMaxBufferedBytes - buffer_.size()) {
        fail("Input buffer limit exceeded");
        return;
    }

    buffer_.append(bytes.data(), bytes.size());
}

void HttpParser::fail(std::string message) {
    if (error_.empty()) {
        error_ = std::move(message);
    }
}

ParseResult HttpParser::errorResult() const {
    return {ParseStatus::Error, std::nullopt, error_};
}

std::optional<std::string_view> HttpParser::readLine() {
    const auto end = buffer_.find("\r\n", cursor_);

    if (end == std::string::npos) {
        if (buffer_.size() - cursor_ >
            kMaxHeaderBytes - header_bytes_) {
            fail("HTTP header section is too large");
        }

        return std::nullopt;
    }

    const std::size_t length = end - cursor_;

    if (length + 2 > kMaxHeaderBytes - header_bytes_) {
        fail("HTTP header section is too large");
        return std::nullopt;
    }

    std::string_view line(buffer_.data() + cursor_, length);

    cursor_ = end + 2;
    header_bytes_ += (length + 2);

    return line;
}

bool HttpParser::parseRequestLine(std::string_view line) {
    const auto first = line.find(' ');

    if (first == std::string_view::npos || first == 0) {
        fail("Invalid HTTP request line");
        return false;
    }

    const auto second = line.find(' ', first + 1);

    if (second == std::string_view::npos ||
        second == first + 1 ||
        second + 1 >= line.size() ||
        line.find(' ', second + 1) != std::string_view::npos) {
        fail("Invalid HTTP request line");
        return false;
    }

    const auto method = line.substr(0, first);
    const auto target = line.substr(first + 1,
                                    second - first - 1);
    const auto version = line.substr(second + 1);

    for (char c : method) {
        if (!isTokenChar(c)) {
            fail("Invalid HTTP method");
            return false;
        }
    }

    if (!validTarget(target)) {
        fail("Invalid request target (expected /path)");
        return false;
    }

    if (version != "HTTP/1.1") {
        fail("Only HTTP/1.1 is supported");
        return false;
    }

    request_.method = method;
    request_.target = target;
    request_.version = version;

    const auto question = target.find('?');

    request_.path = target.substr(0, question);

    if (question != std::string_view::npos) {
        request_.query = target.substr(question + 1);
    }

    return true;
}

bool HttpParser::parseHeader(std::string_view line) {
    const auto colon = line.find(':');

    if (colon == std::string_view::npos || colon == 0) {
        fail("Invalid HTTP header");
        return false;
    }

    const auto name = line.substr(0, colon);

    for (char c : name) {
        if (!isTokenChar(c)) {
            fail("Invalid HTTP header name");
            return false;
        }
    }

    const auto raw_value = line.substr(colon + 1);

    for (unsigned char c : raw_value) {
        if ((c < 0x20 && c != '\t') || c == 0x7f) {
            fail("Invalid control character in header value");
            return false;
        }
    }

    const std::string key = lowercase(name);
    const std::string value(trimOws(raw_value));

    auto it = request_.headers.find(key);

    if (it != request_.headers.end()) {
        if (key == "content-length" || key == "host") {
            fail("Duplicate Host or Content-Length header");
            return false;
        }

        it->second += "," + value;
    } else {
        request_.headers.emplace(key, value);
    }

    return true;
}

bool HttpParser::finishHeaders() {
    const auto host = request_.headers.find("host");

    if (host == request_.headers.end() ||
        host->second.empty() ||
        host->second.find_first_of(" \t,") !=
            std::string::npos) {
        fail("Missing or invalid Host header");
        return false;
    }

    if (request_.headers.contains("transfer-encoding")) {
        fail("Transfer-Encoding is unsupported; use Content-Length");
        return false;
    }

    if (request_.headers.contains("expect")) {
        fail("Expect requests are unsupported");
        return false;
    }

    const auto length = request_.headers.find("content-length");

    if (length != request_.headers.end()) {
        if (length->second.empty()) {
            fail("Empty Content-Length");
            return false;
        }

        const char* first = length->second.data();
        const char* last = first + length->second.size();

        const auto [ptr, ec] =
            std::from_chars(first, last, content_length_);

        if (ec != std::errc{} || ptr != last) {
            fail("Invalid Content-Length");
            return false;
        }

        if (content_length_ > kMaxBodyBytes) {
            fail("Request body is too large");
            return false;
        }
    }

    const auto conn = request_.headers.find("connection");

    request_.keep_alive =
        conn == request_.headers.end() ||
        !hasConnectionToken(conn->second, "close");

    return true;
}

ParseResult HttpParser::finishRequest() {
    request_.body.assign(buffer_.data() + cursor_,
                         content_length_);

    cursor_ += content_length_;

    HttpRequest result = std::move(request_);

    buffer_.erase(0, cursor_);

    cursor_ = 0;
    header_bytes_ = 0;
    content_length_ = 0;

    request_ = HttpRequest{};
    state_ = State::RequestLine;

    return {ParseStatus::Complete, std::move(result), {}};
}

ParseResult HttpParser::parseNext() {
    if (!error_.empty()) {
        return errorResult();
    }

    for (;;) {
        switch (state_) {

            case State::RequestLine: {
                const auto line = readLine();

                if (!error_.empty()) {
                    return errorResult();
                }

                if (!line) {
                    return {};
                }

                if (!parseRequestLine(*line)) {
                    return errorResult();
                }

                state_ = State::Headers;
                break;
            }

            case State::Headers: {
                const auto line = readLine();

                if (!error_.empty()) {
                    return errorResult();
                }

                if (!line) {
                    return {};
                }

                if (!line->empty()) {
                    if (!parseHeader(*line)) {
                        return errorResult();
                    }
                } else {
                    if (!finishHeaders()) {
                        return errorResult();
                    }

                    if (content_length_ == 0) {
                        return finishRequest();
                    }

                    state_ = State::Body;
                }

                break;
            }

            case State::Body: {
                if (buffer_.size() - cursor_ <
                    content_length_) {
                    return {};
                }

                return finishRequest();
            }
        }
    }
}

void HttpParser::reset() {
    buffer_.clear();

    cursor_ = 0;
    header_bytes_ = 0;
    content_length_ = 0;

    state_ = State::RequestLine;

    request_ = HttpRequest{};
    error_.clear();
}

}  // namespace http_server
