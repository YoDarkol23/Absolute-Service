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
#include <regex>

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

std::string handle_admin_request(const HttpRequest& request) {
    // Логируем админский запрос
    std::cout << "[ADMIN] Method: " << request.method << ", Path: " << request.path << std::endl;
    
    // Простая маршрутизация для админов
    if (request.method == "GET" && request.path == "/status") {
        return create_http_response(
            R"({"status": "success", "message": "Server is running", "clients": "active"})"
        );
    }
    else if (request.method == "POST" && request.path == "/reload") {
        // Заглушка для перезагрузки данных
        return create_http_response(
            R"({"status": "success", "message": "Data reloaded"})"
        );
    }
    else {
        return create_http_response(
            R"({"error": "Admin endpoint not found. Try GET /status or POST /reload"})", 
            404
        );
    }
}

std::string handle_client_request(const HttpRequest& request) {
    // Обрабатываем только GET запросы для клиентов
    if (request.method != "GET") {
        return create_http_response(
            R"({"error": "Method not allowed"})", 
            405
        );
    }
    
    // Маршрутизация клиентских запросов
    if (request.path == "/cars" || request.path == "/cars/" || request.path.find("/cars?") == 0) {
        std::string cars_data = handle_get_cars();
        return create_http_response(cars_data);
    }
    else if (request.path == "/" || request.path == "/health") {
        return create_http_response(
            R"({"status": "ok", "service": "CarDelivery API"})"
        );
    }
    else {
        return create_http_response(
            R"({"error": "Endpoint not found. Try GET /cars"})", 
            404
        );
    }
}