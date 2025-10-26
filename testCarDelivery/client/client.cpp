#include "client.hpp"
#include "../common/utils.hpp"
#include <string>

/**
 * Отправляет запрос на получение списка всех автомобилей.
 * 
 * @param host — адрес сервера (например, "127.0.0.1")
 * @param port — порт сервера (обычно 8080)
 * @return JSON-строка с данными или ошибкой
 */
std::string fetch_all_cars(const std::string& host, int port) {
    // Формируем корректный HTTP-запрос
    std::string request = 
        "GET /cars HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";

    // Используем общую функцию из common/utils.cpp
    return send_http_request(host, port, request);
}

/**
 * Отправляет админ-запрос (например, для добавления авто).
 * Пока заглушка — можно расширить позже.
 */
std::string send_admin_request(const std::string& host, int port, const std::string& json_body) {
    std::string request = 
        "POST /admin/update HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: " + std::to_string(json_body.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" + json_body;

    return send_http_request(host, port, request);
}