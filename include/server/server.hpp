
#pragma once

#include "server/config.hpp"

namespace http_server {

class Server {
public:
    explicit Server(Config config);
    int run();

private:
    Config config_;
};

}  // namespace http_server
