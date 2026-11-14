
#include "server/server.hpp"

#include <iostream>

namespace http_server {

Server::Server(Config config) : config_(config) {}

int Server::run() {
    std::cout << "Hello world!\n"
              << "Variant: 5 (lock-free MPMC queue)\n"
              << "Port: " << config_.port << '\n'
              << "Threads: " << config_.threads << '\n';
    return 0;
}

}  // namespace http_server
