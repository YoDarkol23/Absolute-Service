#include "server.hpp"
#include "../common/logger.hpp"
#include <iostream>

int main() {
    try {
         // Initialize logger
        Logger::init();

        // Log server startup
        Logger::log_info("Server starting on port 8080");

        // Создаём сервер: обычный порт = 8080
        CarDeliveryServer server(8080);
        
        std::cout << "Сервер запущен!\n";
        std::cout << "   Обычные запросы: http://localhost:8080\n";
        std::cout << "Ожидание подключений...\n";

        // Запускаем цикл приёма соединений
        server.run();
    } catch (const std::exception& e) {
        std::cerr << " Критическая ошибка: " << e.what() << std::endl;
           Logger::log_error("Critical server error: " + std::string(e.what()));
        return 1;
    }
    return 0;
}