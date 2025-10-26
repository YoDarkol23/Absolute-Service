#pragma once
#include <string>

/**
 * Получает список всех автомобилей с сервера.
 */
std::string fetch_all_cars(const std::string& host = "127.0.0.1", int port = 8080);

/**
 * Отправляет админ-запрос на сервер (порт 8081).
 */
std::string send_admin_request(const std::string& host, int port, const std::string& json_body);