#pragma once
#include <string>

/**
 * Обрабатывает GET /cars — возвращает список автомобилей из data/cars.json
 */
std::string handle_get_cars();

/**
 * Обрабатывает админские запросы (порт 8081).
 * Пока просто проверяет наличие "action" в теле запроса.
 */
std::string handle_admin_request(const std::string& request);
std::string handle_admin_request(const HttpRequest& request);
std::string handle_client_request(const HttpRequest& request);
