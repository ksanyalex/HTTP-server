
#include "server/config.hpp"

#include <charconv>
#include <stdexcept>
#include <string>
#include <string_view>

namespace http_server {
namespace {

unsigned int parse_positive(std::string_view text, std::string_view option) {
    unsigned int number = 0;
    const auto [end, error] =
        std::from_chars(text.data(), text.data() + text.size(), number);

    if (error != std::errc{} || end != text.data() + text.size() || number == 0) {
        throw std::invalid_argument("Invalid value for " + std::string(option));
    }
    return number;
}

}  // namespace

Config parse_config(int argc, char* argv[]) {
    Config config;

    for (int i = 1; i < argc; ++i) {
        const std::string_view option = argv[i];
        if (option != "--port" && option != "--threads") {
            throw std::invalid_argument("Unknown option: " + std::string(option));
        }
        if (i + 1 >= argc) {
            throw std::invalid_argument("Missing value for " + std::string(option));
        }

        const unsigned int value = parse_positive(argv[++i], option);
        if (option == "--port") {
            if (value > 65535) {
                throw std::invalid_argument("Port must be between 1 and 65535");
            }
            config.port = static_cast<std::uint16_t>(value);
        } else {
            config.threads = value;
        }
    }
    return config;
}

}  // namespace http_server
