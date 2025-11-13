
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

// === Интерфейс пользователя ===

void display_main_menu();
void handle_user_choice(int choice);
void display_response(const std::string& response);
