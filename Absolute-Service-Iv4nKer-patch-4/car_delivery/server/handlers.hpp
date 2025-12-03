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
std::string handle_post_calculate_delivery(const std::string& body);

// Эндпоинты админки
std::string handle_post_admin_login(const std::string& body);
std::string handle_get_admin_cars();
std::string handle_post_admin_cars(const std::string& body);
std::string handle_put_admin_cars(int car_id, const std::string& body);
std::string handle_delete_admin_cars(int car_id);
std::string handle_get_admin_cities();
std::string handle_post_admin_cities(const std::string& body);
std::string handle_put_admin_cities(int city_id, const std::string& body);
std::string handle_delete_admin_cities(int city_id);
std::string handle_get_admin_documents();
std::string handle_post_admin_documents(const std::string& body);
std::string handle_delete_admin_documents(const std::string& body);