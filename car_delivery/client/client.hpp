#pragma once
#include <string>

// === HTTP-запросы к серверу ===
std::string fetch_all_cars(const std::string& host = "127.0.0.1", int port = 8080);
std::string fetch_cars_by_specs(const std::string& specs, const std::string& host = "127.0.0.1", int port = 8080);
std::string fetch_cars_by_brand_model(const std::string& brand, const std::string& model, const std::string& host = "127.0.0.1", int port = 8080);
std::string fetch_delivery_cities(const std::string& host = "127.0.0.1", int port = 8080);
std::string fetch_required_documents(const std::string& host = "127.0.0.1", int port = 8080);
std::string fetch_delivery_process(const std::string& host = "127.0.0.1", int port = 8080);
std::string fetch_admin_login(const std::string& username, const std::string& password, const std::string& host = "127.0.0.1", int port = 8080);
std::string fetch_delivery_calculation(int car_id, int city_id, const std::string& host = "127.0.0.1", int port = 8080);

// === Функции вывода ===
void print_car_table(const json& cars);
void print_cities_table(const json& cities);
void print_documents_list(const json& documents);
void print_delivery_process(const json& process);
void print_search_results(const json& results);
void print_admin_login_result(const json& result);
void print_delivery_calculation(const json& calculation); // Добавить эту строку

// === Интерфейс пользователя ===
void display_main_menu();
void handle_user_choice(int choice);
void display_response(const std::string& response);