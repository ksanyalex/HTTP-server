#pragma once

#include "server/http_request.hpp"

#include <string>
#include <utility>
#include <vector>

namespace http_server {
    
    struct HttpResponse {
        int status = 200;
        std::string body;
        bool keep_alive = true;

        std::vector<std::pair<std::string, std::string>> headers;

        std::string serialize() const;

    };

    HttpResponse handleRequest(const HttpRequest& request);

} // namespace http_server