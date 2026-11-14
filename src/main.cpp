
#include "server/config.hpp"
#include "server/server.hpp"

#include <exception>
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        http_server::Server server(http_server::parse_config(argc, argv));
        return server.run();
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n'
                  << "Usage: ./build/server --port 8080 --threads 8\n";
        return 1;
    }
}
