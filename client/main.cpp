#include <iostream>
#include <string>
#include "client.hpp"
#include "utils.hpp"

using json = nlohmann::json;

int main() {
    const std::string host = "127.0.0.1";
    const int port = 8080;
    int choice;
    do {
        std::cout << "\n========================================\n"
                  << "        СИСТЕМА ДОСТАВКИ АВТОМОБИЛЕЙ        \n"
                  << "        \"ABSOLUT SERVICE\"              \n"
                  << "========================================\n"
                  << "1. Просмотреть список автомобилей\n"
                  << "2. Поиск автомобилей по техническим характеристикам\n"
                  << "3. Поиск по марке и моделе\n"
                  << "4. Города доставки\n"
                  << "5. Рассчитать стоимость доставки\n"
                  << "6. Список требуемых документов\n"
                  << "7. Процесс доставки\n"
                  << "8. Вход в режим администратора\n"
                  << "0. Выход\n"
                  << "========================================\n"
                  << "Введите свой выбор: ";
        std::cin >> choice;
        std::cin.ignore();

        std::string response;
        switch (choice) {
            case 1:
                std::cout << "\nПолучение списка автомобилей...\n";
                response = fetch_all_cars(host, port);
                display_response(response);
                std::cout << "\nХотите рассчитать стоимость доставки для какого-либо автомобиля? (y/n): ";
                char choice_calc;
                std::cin >> choice_calc;
                if (choice_calc == 'y' || choice_calc == 'Y') {
                    std::cout << "\n=== СПИСОК ГОРОДОВ ДОСТАВКИ ===\n";
                    std::string cities_response = fetch_delivery_cities(host, port);
                    display_response(cities_response);
                    std::cout << "\nВведите ID автомобиля: ";
                    int car_id;
                    std::cin >> car_id;
                    std::cout << "Введите ID города: ";
                    int city_id;
                    std::cin >> city_id;
                    std::cin.ignore();

                    response = fetch_delivery_calculation(car_id, city_id, host, port);
                    display_response(response);
                }
                break;

            case 2: {
                std::cout << "\n=== ПОИСК АВТОМОБИЛЕЙ ПО ХАРАКТЕРИСТИКАМ ===\n";
                std::cout << "Введите параметры поиска (оставьте пустым, чтобы пропустить):\n";

                std::string brand, model, steering_wheel, fuel_type, country;
                int min_year = 0, max_year = 0, min_horsepower = 0, max_horsepower = 0;
                double min_engine = 0.0, max_engine = 0.0;
                int min_price = 0, max_price = 0;

                std::cout << "Марка: ";
                std::getline(std::cin, brand);

                std::cout << "Модель: ";
                std::getline(std::cin, model);

                std::cout << "Руль (left/right): ";
                std::getline(std::cin, steering_wheel);

                std::cout << "Тип топлива (petrol/diesel/hybrid): ";
                std::getline(std::cin, fuel_type);

                std::cout << "Страна: ";
                std::getline(std::cin, country);

                std::cout << "Минимальный год выпуска: ";
                std::string year_input;
                std::getline(std::cin, year_input);
                if (!year_input.empty()) min_year = std::stoi(year_input);

                std::cout << "Максимальный год выпуска: ";
                std::getline(std::cin, year_input);
                if (!year_input.empty()) max_year = std::stoi(year_input);

                std::cout << "Минимальная мощность (л.с.): ";
                std::string hp_input;
                std::getline(std::cin, hp_input);
                if (!hp_input.empty()) min_horsepower = std::stoi(hp_input);

                std::cout << "Максимальная мощность (л.с.): ";
                std::getline(std::cin, hp_input);
                if (!hp_input.empty()) max_horsepower = std::stoi(hp_input);

                std::cout << "Минимальный объем двигателя (л): ";
                std::string engine_input;
                std::getline(std::cin, engine_input);
                if (!engine_input.empty()) min_engine = std::stod(engine_input);

                std::cout << "Максимальный объем двигателя (л): ";
                std::getline(std::cin, engine_input);
                if (!engine_input.empty()) max_engine = std::stod(engine_input);

                std::cout << "Минимальная цена ($): ";
                std::string price_input;
                std::getline(std::cin, price_input);
                if (!price_input.empty()) min_price = std::stoi(price_input);

                std::cout << "Максимальная цена ($): ";
                std::getline(std::cin, price_input);
                if (!price_input.empty()) max_price = std::stoi(price_input);

                // Формируем строку параметров для поиска
                std::stringstream specs;

                if (!brand.empty()) specs << "brand=" << brand << ",";
                if (!model.empty()) specs << "model=" << model << ",";
                if (!steering_wheel.empty()) specs << "steering_wheel=" << steering_wheel << ",";
                if (!fuel_type.empty()) specs << "fuel_type=" << fuel_type << ",";
                if (!country.empty()) specs << "country=" << country << ",";
                if (min_year > 0) specs << "year>=" << min_year << ",";
                if (max_year > 0) specs << "year<=" << max_year << ",";
                if (min_horsepower > 0) specs << "horsepower>=" << min_horsepower << ",";
                if (max_horsepower > 0) specs << "horsepower<=" << max_horsepower << ",";
                if (min_engine > 0) specs << "engine_volume>=" << min_engine << ",";
                if (max_engine > 0) specs << "engine_volume<=" << max_engine << ",";
                if (min_price > 0) specs << "price_usd>=" << min_price << ",";
                if (max_price > 0) specs << "price_usd<=" << max_price << ",";

                std::string specs_str = specs.str();
                // Удаляем последнюю запятую если есть
                if (!specs_str.empty() && specs_str.back() == ',') {
                    specs_str.pop_back();
                }

                if (specs_str.empty()) {
                    std::cout << "Не указаны параметры для поиска. Показать все автомобили? (y/n): ";
                    char show_all;
                    std::cin >> show_all;
                    if (show_all == 'y' || show_all == 'Y') {
                        response = fetch_all_cars(host, port);
                        display_response(response);
                    }
                }
                else {
                    std::cout << "\nПараметры поиска: " << specs_str << std::endl;
                    response = fetch_cars_by_specs(specs_str, host, port);
                    display_response(response);
                }

                std::cout << "\nХотите рассчитать стоимость доставки для какого-либо автомобиля? (y/n): ";
                char choice_calc2;
                std::cin >> choice_calc2;
                if (choice_calc2 == 'y' || choice_calc2 == 'Y') {\
                    std::cout << "\n=== СПИСОК ГОРОДОВ ДОСТАВКИ ===\n";
                    std::string cities_response = fetch_delivery_cities(host, port);
                    display_response(cities_response);
                    std::cout << "\nВведите ID автомобиля: ";
                    int car_id;
                    std::cin >> car_id;
                    std::cout << "Введите ID города: ";
                    int city_id;
                    std::cin >> city_id;
                    std::cin.ignore();

                    response = fetch_delivery_calculation(car_id, city_id, host, port);
                    display_response(response);
                }
                break;
            }

            case 3: {
                std::cout << "\nМарка: ";
                std::string brand, model;
                std::getline(std::cin, brand);
                std::cout << "Модель (опционально): ";
                std::getline(std::cin, model);
                response = fetch_cars_by_brand_model(brand, model, host, port);
                display_response(response);
                std::cout << "\nХотите рассчитать стоимость доставки для какого-либо автомобиля? (y/n): ";
                char choice_calc3;
                std::cin >> choice_calc3;
                if (choice_calc3 == 'y' || choice_calc3 == 'Y') {
                    std::cout << "\n=== СПИСОК ГОРОДОВ ДОСТАВКИ ===\n";
                    std::string cities_response = fetch_delivery_cities(host, port);
                    display_response(cities_response);
                    std::cout << "\nВведите ID автомобиля: ";
                    int car_id;
                    std::cin >> car_id;
                    std::cout << "Введите ID города: ";
                    int city_id;
                    std::cin >> city_id;
                    std::cin.ignore();

                    response = fetch_delivery_calculation(car_id, city_id, host, port);
                    display_response(response);
                }
                break;
            }

            case 4:
                std::cout << "\nПолучение списка городов...\n";
                response = fetch_delivery_cities(host, port);
                display_response(response);
                break;

            case 5: {
                std::cout << "\nРассчитать стоимость доставки\n";
                std::cout << "Введите ID автомобиля: ";
                int car_id;
                std::cin >> car_id;
                std::cout << "Введите ID города: ";
                int city_id;
                std::cin >> city_id;
                std::cin.ignore();

                response = fetch_delivery_calculation(car_id, city_id, host, port);
                display_response(response);
                break;
            }
            case 6:
                std::cout << "\nПолучение списка требуемых документов...\n";
                response = fetch_required_documents(host, port);
                display_response(response);
                break;

            case 7:
                std::cout << "\nПолучение информации о процессе доставки...\n";
                response = fetch_delivery_process(host, port);
                display_response(response);
                break;

            case 8: {
                std::cout << "\nВход в режим администратора\nЛогин: ";
                std::string username, password;
                std::getline(std::cin, username);
                std::cout << "Пароль: ";
                std::getline(std::cin, password);
                response = fetch_admin_login(username, password, host, port);
                
                try {
                    std::string json_body = extract_json_from_response(response);
                    auto j = json::parse(json_body);
                    
                    if (j.value("status", "") == "success") {
                        std::cout << "\n Аутентификация прошла успешно!\n";
                        std::cout << "Пользователь: " << j["user"].value("username", "unknown") 
                                  << " (" << j["user"].value("role", "user") << ")\n";
                        
                        // После успешного входа открываем админское меню на клиенте
                        int admin_choice;
                        do {
                            display_admin_menu();
                            std::cin >> admin_choice;
                            std::cin.ignore();

                            if (admin_choice != 0) {
                                handle_admin_choice(admin_choice, host, port);
                            }
                        } while (admin_choice != 0);
                    } else {
                        std::cout << "Ошибка аутентификации: " << j.value("error", "Unknown error") << "\n";
                    }
                } catch (...) {
                    std::cout << "Ошибка аутентификации.\n";
                }
                break;
            }

            case 0:
                std::cout << "\nВыход из системы....\n";
                break;

            default:
                std::cout << "Недопустимый выбор. Пожалуйста, попробуйте снова.\n";
        }
    } while (choice != 0);

    return 0;
}