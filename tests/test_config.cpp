#include "server/config.hpp"
#include <iostream>
#include <cassert>
#include <cstring>

void test_po_umnolchaniyu() {
    char* argv[] = {(char*)"server"};
    auto cfg = http_server::parse_config(1, argv);

    if (cfg.port != 8080) {
        std::cerr << "Ошибка: порт по умолчанию должен быть 8080, а не "
                  << cfg.port << std::endl;
        exit(1);
    }
    if (cfg.threads != 8) {
        std::cerr << "Ошибка: число потоков по умолчанию должно быть 8, а не "
                  << cfg.threads << std::endl;
        exit(1);
    }
    std::cout << "[OK] Тест значений по умолчанию пройден" << std::endl;
}

void test_svoi_znacheniya() {
    char* argv[] = {(char*)"server", (char*)"--port", (char*)"9090",
                    (char*)"--threads", (char*)"4"};
    auto cfg = http_server::parse_config(5, argv);

    if (cfg.port != 9090) {
        std::cerr << "Ошибка: порт должен быть 9090" << std::endl;
        exit(1);
    }
    if (cfg.threads != 4) {
        std::cerr << "Ошибка: число потоков должно быть 4" << std::endl;
        exit(1);
    }
    std::cout << "[OK] Тест своих значений пройден" << std::endl;
}

void test_neizvestny_flag() {
    char* argv[] = {(char*)"server", (char*)"--foo"};
    bool caught = false;
    try {
        http_server::parse_config(2, argv);
    } catch (const std::invalid_argument& e) {
        caught = true;
    }
    if (!caught) {
        std::cerr << "Ошибка: должен был выбросить исключение на --foo"
                  << std::endl;
        exit(1);
    }
    std::cout << "[OK] Тест неизвестного флага пройден" << std::endl;
}

void test_flag_bez_znacheniya() {
    char* argv[] = {(char*)"server", (char*)"--port"};
    bool caught = false;
    try {
        http_server::parse_config(2, argv);
    } catch (const std::invalid_argument& e) {
        caught = true;
    }
    if (!caught) {
        std::cerr << "Ошибка: должен был выбросить исключение, "
                     "если у флага нет значения" << std::endl;
        exit(1);
    }
    std::cout << "[OK] Тест флага без значения пройден" << std::endl;
}

void test_bolshoy_port() {
    char* argv[] = {(char*)"server", (char*)"--port", (char*)"70000"};
    bool caught = false;
    try {
        http_server::parse_config(3, argv);
    } catch (const std::invalid_argument& e) {
        caught = true;
    }
    if (!caught) {
        std::cerr << "Ошибка: порт 70000 должен быть отклонён" << std::endl;
        exit(1);
    }
    std::cout << "[OK] Тест большого порта пройден" << std::endl;
}

void test_nol_potokov() {
    char* argv[] = {(char*)"server", (char*)"--threads", (char*)"0"};
    bool caught = false;
    try {
        http_server::parse_config(3, argv);
    } catch (const std::invalid_argument& e) {
        caught = true;
    }
    if (!caught) {
        std::cerr << "Ошибка: 0 потоков должен быть отклонён" << std::endl;
        exit(1);
    }
    std::cout << "[OK] Тест нуля потоков пройден" << std::endl;
}

int main() {
    std::cout << "=== Запуск тестов конфигурации ===" << std::endl;

    test_po_umnolchaniyu();
    test_svoi_znacheniya();
    test_neizvestny_flag();
    test_flag_bez_znacheniya();
    test_bolshoy_port();
    test_nol_potokov();

    std::cout << "=== Все тесты прошли успешно ===" << std::endl;
    return 0;
}