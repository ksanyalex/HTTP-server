#include "server/config.hpp"
#include <iostream>
#include <cassert>
#include <cstring>
#include <stdexcept>
#include <cstdlib>

void test_defaults() {
    char* argv[] = {(char*)"server"};
    auto cfg = http_server::parse_config(1, argv);

    if (cfg.port != 8080) {
        std::cerr << "Error: default port should be 8080, got "
                  << cfg.port << std::endl;
        exit(1);
    }
    if (cfg.threads != 8) {
        std::cerr << "Error: default threads should be 8, got "
                  << cfg.threads << std::endl;
        exit(1);
    }
    std::cout << "[OK] Default values test passed" << std::endl;
}

void test_custom_values() {
    char* argv[] = {(char*)"server", (char*)"--port", (char*)"9090",
                    (char*)"--threads", (char*)"4"};
    auto cfg = http_server::parse_config(5, argv);

    if (cfg.port != 9090) {
        std::cerr << "Error: port should be 9090" << std::endl;
        exit(1);
    }
    if (cfg.threads != 4) {
        std::cerr << "Error: threads should be 4" << std::endl;
        exit(1);
    }
    std::cout << "[OK] Custom values test passed" << std::endl;
}

void test_unknown_flag() {
    char* argv[] = {(char*)"server", (char*)"--foo"};
    bool caught = false;
    try {
        http_server::parse_config(2, argv);
    } catch (const std::invalid_argument& e) {
        caught = true;
    }
    if (!caught) {
        std::cerr << "Error: should throw exception for --foo"
                  << std::endl;
        exit(1);
    }
    std::cout << "[OK] Unknown flag test passed" << std::endl;
}

void test_missing_value() {
    char* argv[] = {(char*)"server", (char*)"--port"};
    bool caught = false;
    try {
        http_server::parse_config(2, argv);
    } catch (const std::invalid_argument& e) {
        caught = true;
    }
    if (!caught) {
        std::cerr << "Error: should throw exception when flag "
                     "has no value" << std::endl;
        exit(1);
    }
    std::cout << "[OK] Missing value test passed" << std::endl;
}

void test_port_too_large() {
    char* argv[] = {(char*)"server", (char*)"--port", (char*)"70000"};
    bool caught = false;
    try {
        http_server::parse_config(3, argv);
    } catch (const std::invalid_argument& e) {
        caught = true;
    }
    if (!caught) {
        std::cerr << "Error: port 70000 should be rejected" << std::endl;
        exit(1);
    }
    std::cout << "[OK] Port too large test passed" << std::endl;
}

void test_zero_threads() {
    char* argv[] = {(char*)"server", (char*)"--threads", (char*)"0"};
    bool caught = false;
    try {
        http_server::parse_config(3, argv);
    } catch (const std::invalid_argument& e) {
        caught = true;
    }
    if (!caught) {
        std::cerr << "Error: 0 threads should be rejected" << std::endl;
        exit(1);
    }
    std::cout << "[OK] Zero threads test passed" << std::endl;
}

int main() {
    std::cout << "=== Running config tests ===" << std::endl;

    test_defaults();
    test_custom_values();
    test_unknown_flag();
    test_missing_value();
    test_port_too_large();
    test_zero_threads();

    std::cout << "=== All tests passed ===" << std::endl;
    return 0;
}