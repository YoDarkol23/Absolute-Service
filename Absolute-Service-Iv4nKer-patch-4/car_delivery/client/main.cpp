
#include <iostream>
#include <string>
#include "client.hpp"

int main() {
    const std::string host = "127.0.0.1";
    const int client_port = 8080; // ← только один порт

    int choice;
    do {
        std::cout << "\n========================================\n"
                  << "        VEHICLE DELIVERY SYSTEM         \n"
                  << "        \"ABSOLUT SERVICE\"              \n"
                  << "========================================\n"
                  << "1. List available vehicles\n"
                  << "2. Search vehicles by specifications\n"
                  << "3. Search by brand and model\n"
                  << "4. Delivery cities\n"
                  << "5. Calculate delivery cost\n"
                  << "6. Required documents\n"
                  << "7. Delivery process\n"
                  << "8. Administrator login\n"
                  << "0. Exit\n"
                  << "========================================\n"
                  << "Enter your choice: ";
        std::cin >> choice;
        std::cin.ignore();

        std::string response;
        switch (choice) {
            case 1:
                std::cout << "\nFetching vehicle list...\n";
                response = fetch_all_cars(host, client_port);
                display_response(response);
                // Offer to calculate delivery for any car
                std::cout << "\nWould you like to calculate delivery cost for any car? (y/n): ";
                char choice_calc;
                std::cin >> choice_calc;
                if (choice_calc == 'y' || choice_calc == 'Y') {
                    std::cout << "\nEnter car ID: ";
                    int car_id;
                    std::cin >> car_id;
                    std::cout << "Enter city ID: ";
                    int city_id;
                    std::cin >> city_id;
                    std::cin.ignore();

                    response = fetch_delivery_calculation(car_id, city_id, host, client_port);
                    display_response(response);
                }
                break;

            case 2: {
                std::cout << "\nEnter search filters (e.g., brand=Toyota,year=2020,horsepower=150): ";
                std::string specs;
                std::getline(std::cin, specs);
                response = fetch_cars_by_specs(specs, host, client_port);
                display_response(response);
                   // Offer to calculate delivery for any car
                std::cout << "\nWould you like to calculate delivery cost for any car? (y/n): ";
                char choice_calc;
                std::cin >> choice_calc;
                if (choice_calc == 'y' || choice_calc == 'Y') {
                    std::cout << "\nEnter car ID: ";
                    int car_id;
                    std::cin >> car_id;
                    std::cout << "Enter city ID: ";
                    int city_id;
                    std::cin >> city_id;
                    std::cin.ignore();

                    response = fetch_delivery_calculation(car_id, city_id, host, client_port);
                    display_response(response);
                }
                break;
            }

            case 3: {
                std::cout << "\nBrand: ";
                std::string brand, model;
                std::getline(std::cin, brand);
                std::cout << "Model (optional): ";
                std::getline(std::cin, model);
                response = fetch_cars_by_brand_model(brand, model, host, client_port); // ← client_port, не admin_port
                display_response(response);
                
                // Offer to calculate delivery for any car
                std::cout << "\nWould you like to calculate delivery cost for any car? (y/n): ";
                char choice_calc;
                std::cin >> choice_calc;
                if (choice_calc == 'y' || choice_calc == 'Y') {
                    std::cout << "\nEnter car ID: ";
                    int car_id;
                    std::cin >> car_id;
                    std::cout << "Enter city ID: ";
                    int city_id;
                    std::cin >> city_id;
                    std::cin.ignore();

                    response = fetch_delivery_calculation(car_id, city_id, host, client_port);
                    display_response(response);
                }
                break;
            }

            case 4:
                std::cout << "\nFetching delivery cities...\n";
                response = fetch_delivery_cities(host, client_port);
                display_response(response);
                break;

            case 5: {
                std::cout << "\nCalculate delivery cost\n";
                std::cout << "Enter car ID: ";
                int car_id;
                std::cin >> car_id;
                std::cout << "Enter city ID: ";
                int city_id;
                std::cin >> city_id;
                std::cin.ignore();

                response = fetch_delivery_calculation(car_id, city_id, host, client_port);
                display_response(response);
                break;
            }
            case 6:
                std::cout << "\nFetching required documents...\n";
                response = fetch_required_documents(host, client_port);
                display_response(response);
                break;

            case 7:
                std::cout << "\nFetching delivery process information...\n";
                response = fetch_delivery_process(host, client_port);
                display_response(response);
                break;

            case 8: {
                std::cout << "\nAdministrator login\nUsername: ";
                std::string username, password;
                std::getline(std::cin, username);
                std::cout << "Password: ";
                std::getline(std::cin, password);
                std::string response = fetch_admin_login(username, password, host, client_port); // ← client_port
                display_response(response);

                // Проверяем успешную аутентификацию и открываем админ-меню
                try {
                    std::string json_body = extract_json_from_response(response);
                    auto j = json::parse(json_body);
                    if (j.value("status", "") == "success") {
                        std::cout << "\nAccess granted. Opening administrator menu...\n";
                        admin_menu(host, client_port);
                    } else {
                        std::cout << "Access denied.\n";
                    }
                } catch (...) {
                    std::cout << "Authentication failed.\n";
                }
                break;
            }

            case 0:
                std::cout << "\nExiting the system.\n";
                break;

            default:
                std::cout << "Invalid option. Please try again.\n";
        }
    } while (choice != 0);

    return 0;
}
