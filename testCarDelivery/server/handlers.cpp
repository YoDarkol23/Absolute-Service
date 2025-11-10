/**
 * @file server/handlers.cpp
 * @brief Реализация бизнес-логики сервера с nlohmann/json.
 */

#include "handlers.hpp"
#include "../common/utils.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <sstream>
#include <algorithm>

using json = nlohmann::json;

// Функция для фильтрации автомобилей по критериям
json filter_cars(const json& cars, const std::unordered_map<std::string, std::string>& filters) {
    json result = json::array();
    
    for (const auto& car : cars) {
        bool matches = true;
        
        for (const auto& [key, value] : filters) {
            if (value.empty()) continue;
            
            if (!car.contains(key)) {
                matches = false;
                break;
            }
            
            std::string car_value;
            if (car[key].is_string()) {
                car_value = car[key].get<std::string>();
            } else if (car[key].is_number()) {
                car_value = std::to_string(car[key].get<double>());
            } else {
                matches = false;
                break;
            }
            
            // Обработка числовых фильтров
            if (key == "year" || key == "price_usd" || key == "engine_volume") {
                try {
                    double car_num = std::stod(car_value);
                    double filter_num = std::stod(value);
                    
                    if (key == "year" && car_num == filter_num) continue;
                    if (key == "price_usd" && car_num <= filter_num) continue;
                    if (key == "engine_volume" && car_num == filter_num) continue;
                    
                    matches = false;
                    break;
                } catch (...) {
                    matches = false;
                    break;
                }
            } else {
                // Строковый поиск (частичное совпадение)
                std::string car_lower = car_value;
                std::string value_lower = value;
                std::transform(car_lower.begin(), car_lower.end(), car_lower.begin(), ::tolower);
                std::transform(value_lower.begin(), value_lower.end(), value_lower.begin(), ::tolower);
                
                if (car_lower.find(value_lower) == std::string::npos) {
                    matches = false;
                    break;
                }
            }
        }
        
        if (matches) {
            result.push_back(car);
        }
    }
    
    return result;
}

std::string handle_get_cars() {
    try {
        std::string content = read_file("data/cars.json");
        if (content.empty()) {
            return R"({"error": "Файл cars.json пуст или не найден"})";
        }
        
        json cars = json::parse(content);
        return cars.dump();
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка обработки cars.json: " << e.what() << std::endl;
        return R"({"error": "Ошибка загрузки данных об автомобилях"})";
    }
}

std::string handle_post_search(const std::string& body) {
    try {
        if (body.empty()) {
            return R"({"error": "Тело запроса пустое"})";
        }
        
        json request = json::parse(body);
        json all_cars = json::parse(read_file("data/cars.json"));
        
        if (!request.contains("filters") || !request["filters"].is_object()) {
            return R"({"error": "Неверный формат запроса. Ожидается объект filters"})";
        }
        
        std::unordered_map<std::string, std::string> filters;
        for (auto& [key, value] : request["filters"].items()) {
            if (value.is_string()) {
                filters[key] = value.get<std::string>();
            } else if (value.is_number()) {
                filters[key] = std::to_string(value.get<double>());
            }
        }
        
        json filtered_cars = filter_cars(all_cars, filters);
        
        json response = {
            {"message", "Результаты поиска"},
            {"found", filtered_cars.size()},
            {"results", filtered_cars}
        };
        
        return response.dump();
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка поиска: " << e.what() << std::endl;
        return R"({"error": "Ошибка выполнения поиска"})";
    }
}

std::string handle_get_search(const std::string& query_string) {
    try {
        std::unordered_map<std::string, std::string> filters;
        std::istringstream iss(query_string);
        std::string pair;
        
        while (std::getline(iss, pair, '&')) {
            size_t pos = pair.find('=');
            if (pos != std::string::npos) {
                std::string key = pair.substr(0, pos);
                std::string value = pair.substr(pos + 1);
                filters[key] = value;
            }
        }
        
        json all_cars = json::parse(read_file("data/cars.json"));
        json filtered_cars = filter_cars(all_cars, filters);
        
        json response = {
            {"message", "Результаты поиска"},
            {"found", filtered_cars.size()},
            {"results", filtered_cars}
        };
        
        return response.dump();
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка GET поиска: " << e.what() << std::endl;
        return R"({"error": "Ошибка выполнения поиска"})";
    }
}

std::string handle_get_cities() {
    try {
        std::string content = read_file("data/cities.json");
        if (!content.empty()) {
            json cities = json::parse(content);
            return cities.dump();
        }
        
        // Возвращаем встроенный список городов
        json cities = json::array();
        cities.push_back({{"id", 1}, {"name", "Москва"}, {"delivery_days", 30}, {"delivery_cost", 1000}});
        cities.push_back({{"id", 2}, {"name", "Санкт-Петербург"}, {"delivery_days", 35}, {"delivery_cost", 1200}});
        cities.push_back({{"id", 3}, {"name", "Нижний Новгород"}, {"delivery_days", 35}, {"delivery_cost", 1200}});
        cities.push_back({{"id", 4}, {"name", "Казань"}, {"delivery_days", 38}, {"delivery_cost", 1250}});
        cities.push_back({{"id", 5}, {"name", "Екатеринбург"}, {"delivery_days", 40}, {"delivery_cost", 1300}});
        
        return cities.dump();
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка загрузки городов: " << e.what() << std::endl;
        return R"({"error": "Ошибка загрузки списка городов"})";
    }
}

std::string handle_get_documents() {
    try {
        std::string content = read_file("data/documents.json");
        if (content.empty()) {
            return R"({"error": "Файл documents.json не найден"})";
        }
        
        json documents = json::parse(content);
        return documents.dump();
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка загрузки документов: " << e.what() << std::endl;
        return R"({"error": "Ошибка загрузки документов"})";
    }
}

std::string handle_get_delivery() {
    try {
        json process = {
            {"process", {
                {{"step", 1}, {"description", "Выбор автомобиля и заключение договора"}, {"status", "completed"}},
                {{"step", 2}, {"description", "Оплата и оформление документов"}, {"status", "completed"}},
                {{"step", 3}, {"description", "Доставка автомобиля в порт назначения"}, {"status", "in_progress"}},
                {{"step", 4}, {"description", "Таможенное оформление"}, {"status", "pending"}},
                {{"step", 5}, {"description", "Постановка на учет в ГИБДД"}, {"status", "pending"}}
            }},
            {"current_step", 3},
            {"progress", 60},
            {"duration", "30-45 дней"},
            {"estimated_completion", "2025-02-15"},
            {"cost", "от 1000 USD"}
        };
        
        return process.dump();
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка формирования информации о доставке: " << e.what() << std::endl;
        return R"({"error": "Ошибка загрузки информации о доставке"})";
    }
}

std::string handle_post_admin_login(const std::string& body) {
    try {
        if (body.empty()) {
            return R"({"error": "Тело запроса пустое"})";
        }
        
        json request = json::parse(body);
        
        std::string username = request.value("username", "");
        std::string password = request.value("password", "");
        
        if (username == "admin" && password == "admin") {
            json response = {
                {"status", "success"},
                {"message", "Вход выполнен успешно"},
                {"token", "admin_token_" + std::to_string(time(nullptr))},
                {"user", {
                    {"username", "admin"},
                    {"role", "administrator"}
                }}
            };
            return response.dump();
        } else {
            return R"({"error": "Неверные учетные данные. Используйте admin/admin"})";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка входа администратора: " << e.what() << std::endl;
        return R"({"error": "Ошибка аутентификации"})";
    }
}

std::string handle_get_cars_specs(const std::string& query_string) {
    return handle_get_search(query_string);
}

std::string handle_get_cars_brand(const std::string& query_string) {
    return handle_get_search(query_string);
}

std::string handle_get_delivery_cities() {
    return handle_get_cities();
}

std::string handle_get_delivery_process() {
    return handle_get_delivery();
}