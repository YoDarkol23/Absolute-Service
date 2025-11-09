/**
 * @file server/handlers.cpp
 * @brief Реализация бизнес-логики сервера.
 * 
 * Содержит обработчики:
 * - GET /cars → возвращает список автомобилей
 * - Админ-запросы → заглушка (готова к расширению)
 * 
 * Все данные читаются из папки ./data/
 */

#include "handlers.hpp"
#include "../common/utils.hpp"
#include <iostream>

std::string handle_get_cars() {
    // Читаем файл с автомобилями
    std::string content = read_file("data/cars.json");
    
    // Если файл пустой или ошибка — возвращаем понятный JSON
    if (content.empty() || content.find("error") != std::string::npos) {
        std::cerr << "⚠️  Не удалось загрузить data/cars.json\n";
        return R"([{"error": "No cars available. Check server data directory."}])";
    }
    
    return content;
}

std::string handle_admin_request(const std::string& request) {
    // Простая проверка: если в теле есть "action", считаем запрос валидным
    if (request.find("\"action\"") != std::string::npos) {
        return R"({"status": "success", "message": "Admin command executed"})";
    }
    return R"({"error": "Invalid admin request. Include \"action\" in JSON body."})";
}