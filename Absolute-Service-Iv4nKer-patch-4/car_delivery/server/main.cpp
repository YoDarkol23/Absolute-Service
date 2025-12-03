#include "server.hpp"
#include <iostream>

/**
 * Главная функция сервера.
 * Запускает сервер на двух портах:
 * - 8080 — для обычных клиентов
 * - 8081 — для администратора
 */
int main() {
    try {
        // Создаём сервер: обычный порт = 8080, админ-порт = 8081
        CarDeliveryServer server(8080);
        
        std::cout << "Сервер запущен!\n";
        std::cout << "   Обычные запросы: http://localhost:8080\n";
        std::cout << "Ожидание подключений...\n";

        // Запускаем цикл приёма соединений
        server.run();
    } catch (const std::exception& e) {
        std::cerr << " Критическая ошибка: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}