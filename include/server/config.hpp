
#pragma once

#include <cstdint>

namespace http_server {

struct Config {
    std::uint16_t port = 8080;
    unsigned int threads = 8;
};

Config parse_config(int argc, char* argv[]);

}  // namespace http_server
