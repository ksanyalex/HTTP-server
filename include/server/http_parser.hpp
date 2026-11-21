
#pragma once

#include "server/http_request.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace server {

enum class ParseStatus {
    Incomplete,
    Complete,
    Error
};

struct ParseResult {
    ParseStatus status = ParseStatus::Incomplete;
    std::optional<HttpRequest> request;
    std::string error;
};

class HttpParser {
public:
    static constexpr std::size_t kMaxHeaderBytes = 16 * 1024;
    static constexpr std::size_t kMaxBodyBytes = 1024 * 1024;
    static constexpr std::size_t kMaxBufferedBytes = 2 * 1024 * 1024;

    void append(std::string_view bytes);

    ParseResult parseNext();

    void reset();

private:
    enum class State {
        RequestLine,
        Headers,
        Body
    };

    std::optional<std::string_view> readLine();

    bool parseRequestLine(std::string_view line);
    bool parseHeader(std::string_view line);
    bool finishHeaders();

    ParseResult finishRequest();
    ParseResult errorResult() const;

    void fail(std::string message);

    std::string buffer_;

    std::size_t cursor_ = 0;
    std::size_t header_bytes_ = 0;
    std::size_t content_length_ = 0;

    State state_ = State::RequestLine;

    HttpRequest request_;
    std::string error_;
};

}  // namespace server
