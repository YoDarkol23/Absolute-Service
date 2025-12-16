#pragma once
#include <string>
#include "json.hpp"
#include "utils.hpp"

using json = nlohmann::json;

// === HTTP-запросы к серверу ===
std::string fetch_all_cars(const std::string& host = "127.0.0.1", int port = 8080);
std::string fetch_cars_by_specs(const std::string& specs, const std::string& host = "127.0.0.1", int port = 8080);
std::string fetch_cars_by_brand_model(const std::string& brand, const std::string& model, const std::string& host = "127.0.0.1", int port = 8080);
std::string fetch_delivery_cities(const std::string& host = "127.0.0.1", int port = 8080);
std::string fetch_required_documents(const std::string& host = "127.0.0.1", int port = 8080);
std::string fetch_delivery_process(const std::string& host = "127.0.0.1", int port = 8080);
std::string fetch_admin_login(const std::string& username, const std::string& password, const std::string& host = "127.0.0.1", int port = 8080);
std::string fetch_delivery_calculation(int car_id, int city_id, const std::string& host = "127.0.0.1", int port = 8080);

// === Админские HTTP-запросы ===
std::string fetch_admin_cars(const std::string& host = "127.0.0.1", int port = 8080);
std::string fetch_admin_cities(const std::string& host = "127.0.0.1", int port = 8080);
std::string fetch_admin_documents(const std::string& host = "127.0.0.1", int port = 8080);
std::string add_admin_car(const std::string& brand, const std::string& model, int year, int price_usd, const std::string& host = "127.0.0.1", int port = 8080);
std::string add_admin_city(const std::string& name, int delivery_days, int delivery_cost, const std::string& host = "127.0.0.1", int port = 8080);
std::string add_admin_document(const std::string& category, const std::string& name, const std::string& host = "127.0.0.1", int port = 8080);
std::string update_admin_car(int car_id, const std::string& brand, const std::string& model, int year, int price_usd, const std::string& host = "127.0.0.1", int port = 8080);
std::string update_admin_city(int city_id, const std::string& name, int delivery_days, int delivery_cost, const std::string& host = "127.0.0.1", int port = 8080);
std::string delete_admin_car(int car_id, const std::string& host = "127.0.0.1", int port = 8080);
std::string delete_admin_city(int city_id, const std::string& host = "127.0.0.1", int port = 8080);
std::string delete_admin_document(const std::string& category, const std::string& name, const std::string& host = "127.0.0.1", int port = 8080);

// === Функции вывода ===
void print_car_table(const json& cars);
void print_cities_table(const json& cities);
void print_documents_list(const json& documents);
void print_delivery_process(const json& process);
void print_search_results(const json& results);
void print_admin_login_result(const json& result);
void print_delivery_calculation(const json& calculation);

// === Админское меню ===
void display_admin_menu();
void handle_admin_choice(int choice, const std::string& host = "127.0.0.1", int port = 8080);

// === Интерфейс пользователя ===
void display_main_menu();
void handle_user_choice(int choice);
void display_response(const std::string& response);