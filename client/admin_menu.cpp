#include <iostream>
#include <string>
#include "client.hpp"

using json = nlohmann::json;

void display_admin_menu() {
    std::cout << "\n========================================\n";
    std::cout << "           МЕНЮ АДМИНИСТРАТОРА           \n";
    std::cout << "========================================\n";
    std::cout << "1. Просмотреть все автомобили\n";
    std::cout << "2. Добавить новый автомобиль\n";
    std::cout << "3. Обновить данные для автомобиля\n";
    std::cout << "4. Удалить автомобиль\n";
    std::cout << "5. Просмотреть все города\n";
    std::cout << "6. Добавить новый город\n";
    std::cout << "7. Обновить данные о городе\n";
    std::cout << "8. Удалить город\n";
    std::cout << "9. Просмотреть все документы\n";
    std::cout << "10. Добавить новый документ\n";
    std::cout << "11. Удалить документ\n";
    std::cout << "0. Покинуть меню администратора\n";
    std::cout << "========================================\n";
    std::cout << "Введите свой выбор: ";
}

void handle_admin_choice(int choice, const std::string& host, int port) {
    std::string response;
    
    switch (choice) {
        case 1:
            std::cout << "\nПолучение данных об автомобилях\n";
            response = fetch_admin_cars(host, port);
            display_response(response);
            break;

        case 2: {
            std::cout << "\nДобавление новго автомобиля\n";
            std::cout << "Марка: ";
            std::string brand, model;
            std::getline(std::cin, brand);
            std::cout << "Модель: ";
            std::getline(std::cin, model);
            std::cout << "Год выпуска: ";
            int year, price_usd;
            std::cin >> year;
            std::cout << "Стоимость (USD): ";
            std::cin >> price_usd;
            std::cin.ignore();

            response = add_admin_car(brand, model, year, price_usd, host, port);
            display_response(response);
            break;
        }

        case 3: {
            std::cout << "\nОбновление данных об автомобиле\n";
            std::cout << "ID автомобиля: ";
            int car_id, year, price_usd;
            std::string brand, model;
            std::cin >> car_id;
            std::cin.ignore();
            std::cout << "Марка: ";
            std::getline(std::cin, brand);
            std::cout << "Модель: ";
            std::getline(std::cin, model);
            std::cout << "Год: ";
            std::cin >> year;
            std::cout << "Стоимость (USD): ";
            std::cin >> price_usd;
            std::cin.ignore();

            response = update_admin_car(car_id, brand, model, year, price_usd, host, port);
            display_response(response);
            break;
        }

        case 4: {
            std::cout << "\nУдаление автомобиля\n";
            std::cout << "Выберите ID автомобиля, которого хотите удалить: ";
            int car_id;
            std::cin >> car_id;
            std::cin.ignore();

            response = delete_admin_car(car_id, host, port);
            display_response(response);
            break;
        }

        case 5:
            std::cout << "\nПолучение данных о городах...\n";
            response = fetch_admin_cities(host, port);
            display_response(response);
            break;

        case 6: {
            std::cout << "\nДобавление города\n";
            std::cout << "Название города: ";
            std::string name;
            std::getline(std::cin, name);
            std::cout << "Сроки доставки (в днях): ";
            int delivery_days, delivery_cost;
            std::cin >> delivery_days;
            std::cout << "Стоимость доставки: ";
            std::cin >> delivery_cost;
            std::cin.ignore();

            response = add_admin_city(name, delivery_days, delivery_cost, host, port);
            display_response(response);
            break;
        }

        case 7: {
            std::cout << "\nОбновление данных о городе\n";
            std::cout << "ID города: ";
            int city_id, delivery_days, delivery_cost;
            std::string name;
            std::cin >> city_id;
            std::cin.ignore();
            std::cout << "Название города: ";
            std::getline(std::cin, name);
            std::cout << "Сроки доставки (в днях): ";
            std::cin >> delivery_days;
            std::cout << "Стоимость доставки: ";
            std::cin >> delivery_cost;
            std::cin.ignore();

            response = update_admin_city(city_id, name, delivery_days, delivery_cost, host, port);
            display_response(response);
            break;
        }

        case 8: {
            std::cout << "\nУдаление города\n";
            std::cout << "Выберите ID города, которого хотите удалить ";
            int city_id;
            std::cin >> city_id;
            std::cin.ignore();

            response = delete_admin_city(city_id, host, port);
            display_response(response);
            break;
        }

        case 9:
            std::cout << "\nПолучение данных о документах...\n";
            response = fetch_admin_documents(host, port);
            display_response(response);
            break;

        case 10: {
            std::cout << "\nДобавление нового документа\n";
            std::cout << "Категория (purchase/registration): ";
            std::string category, name;
            std::getline(std::cin, category);
            std::cout << "Название документа: ";
            std::getline(std::cin, name);

            response = add_admin_document(category, name, host, port);
            display_response(response);
            break;
        }

        case 11: {
            std::cout << "\nУдаление документа\n";
            std::cout << "Категория (purchase/registration): ";
            std::string category, name;
            std::getline(std::cin, category);
            std::cout << "Название документа: ";
            std::getline(std::cin, name);

            response = delete_admin_document(category, name, host, port);
            display_response(response);
            break;
        }

        case 0:
            std::cout << "\nВыход из меню администратора.\n";
            break;

        default:
            std::cout << "Недопустимый выбор. Пожалуйста, попробуйте снова.\n";
    }
}