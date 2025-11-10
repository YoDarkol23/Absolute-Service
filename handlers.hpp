/**
 * @file server/handlers.hpp
 * @brief Объявления обработчиков HTTP-запросов.
 * 
 * Каждая функция соответствует одному эндпоинту сервера.
 * Реализация находится в handlers.cpp.
 */

#pragma once
#include <string>

// Эндпоинты клиентской части
std::string handle_get_cars();
std::string handle_post_search(const std::string& body);
std::string handle_get_search(const std::string& query_string);
std::string handle_get_cities();
std::string handle_get_documents();
std::string handle_get_delivery();

// Дополнительные эндпоинты для клиента
std::string handle_get_cars_specs(const std::string& query_string);
std::string handle_get_cars_brand(const std::string& query_string);
std::string handle_get_delivery_cities();
std::string handle_get_delivery_process();

// Эндпоинт админки
std::string handle_post_admin_login(const std::string& body);