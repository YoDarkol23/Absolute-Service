// server/main.cpp
#include "server.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    try {
        CarDeliveryServer server(8080, 8081);
        
        std::cout << "🚀 Сервер запущен!\n";
        std::cout << "   Обычные запросы: http://localhost:8080\n";
        std::cout << "   Админка:         http://localhost:8081\n";
        std::cout << "Ожидание подключений...\n";

        // Запускаем сервер (неблокирующий)
        server.run();

        // Держим основную программу активной
        std::cout << "Сервер запущен в фоновом режиме. Для остановки нажмите Ctrl+C\n";
        
        // Бесконечный цикл, чтобы программа не завершалась
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

    } catch (const std::exception& e) {
        std::cerr << "❌ Критическая ошибка: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}