// handlers.hpp (обновлённый)
#pragma once
#include <string>
#include "http_parser.hpp"

/**
 * Обрабатывает GET /cars — возвращает список автомобилей из data/cars.json
 */
HttpResponse handle_get_cars();

/**
 * Обрабатывает админские запросы (порт 8081).
 */
HttpResponse handle_admin_request(const HttpRequest& request);

/**
 * Обрабатывает клиентские запросы (порт 8080).
 */
HttpResponse handle_client_request(const HttpRequest& request);