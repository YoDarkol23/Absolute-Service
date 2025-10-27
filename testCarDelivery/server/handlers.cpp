// handlers.cpp (обновлённый)
#include "handlers.hpp"
#include "../common/utils.hpp"
#include <iostream>

HttpResponse handle_get_cars() {
    std::string content = read_file("data/cars.json");
    
    if (content.empty() || content.find("error") != std::string::npos) {
        std::cerr << "⚠️  Не удалось загрузить data/cars.json\n";
        return HttpResponse::error_response(500, "No cars available. Check server data directory.");
    }
    
    return HttpResponse::json_response(content);
}

HttpResponse handle_admin_request(const HttpRequest& request) {
    // Проверяем метод
    if (request.method != "POST") {
        return HttpResponse::error_response(405, "Method Not Allowed. Use POST for admin requests.");
    }
    
    // Проверяем Content-Type
    if (!request.is_json_content()) {
        return HttpResponse::error_response(400, "Content-Type must be application/json");
    }
    
    // Проверяем наличие action в теле
    if (request.body.find("\"action\"") == std::string::npos) {
        return HttpResponse::error_response(400, "Invalid admin request. Include \"action\" in JSON body.");
    }
    
    return HttpResponse::json_response(R"({"status": "success", "message": "Admin command executed"})");
}

HttpResponse handle_client_request(const HttpRequest& request) {
    if (request.method == "GET" && request.path == "/cars") {
        return handle_get_cars();
    }
    
    if (request.method == "GET" && request.path == "/") {
        return HttpResponse::json_response(R"({"message": "Car Delivery Server API", "endpoints": ["GET /cars"]})");
    }
    
    return HttpResponse::error_response(404, "Endpoint not supported. Try GET /cars");
}