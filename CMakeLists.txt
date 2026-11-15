
cmake_minimum_required(VERSION 3.16)
project(HTTPServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_executable(server
    src/main.cpp
    src/config.cpp
    src/server.cpp
)

target_include_directories(server PRIVATE include)
target_link_libraries(server PRIVATE Threads::Threads)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Тесты конфигурации (Участник 2, Неделя 1)
enable_testing()

add_executable(test_config tests/test_config.cpp src/config.cpp)
target_include_directories(test_config PRIVATE include)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_config PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME test_config COMMAND test_config)