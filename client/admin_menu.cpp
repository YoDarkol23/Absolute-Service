#include <iostream>
#include <string>
#include "client.hpp"

using json = nlohmann::json;

void display_admin_menu() {
    std::cout << "\n========================================\n";
    std::cout << "           ADMINISTRATOR MENU           \n";
    std::cout << "========================================\n";
    std::cout << "1. View all cars\n";
    std::cout << "2. Add new car\n";
    std::cout << "3. Update car\n";
    std::cout << "4. Delete car\n";
    std::cout << "5. View all cities\n";
    std::cout << "6. Add new city\n";
    std::cout << "7. Update city\n";
    std::cout << "8. Delete city\n";
    std::cout << "9. View all documents\n";
    std::cout << "10. Add new document\n";
    std::cout << "11. Delete document\n";
    std::cout << "0. Exit admin menu\n";
    std::cout << "========================================\n";
    std::cout << "Enter your choice: ";
}

void handle_admin_choice(int choice, const std::string& host, int port) {
    std::string response;
    
    switch (choice) {
        case 1: // View all cars
            std::cout << "\nFetching all cars...\n";
            response = fetch_admin_cars(host, port);
            display_response(response);
            break;

        case 2: { // Add new car
            std::cout << "\nAdding new car\n";
            std::cout << "Brand: ";
            std::string brand, model;
            std::getline(std::cin, brand);
            std::cout << "Model: ";
            std::getline(std::cin, model);
            std::cout << "Year: ";
            int year, price_usd;
            std::cin >> year;
            std::cout << "Price (USD): ";
            std::cin >> price_usd;
            std::cin.ignore();

            response = add_admin_car(brand, model, year, price_usd, host, port);
            display_response(response);
            break;
        }

        case 3: { // Update car
            std::cout << "\nUpdating car\n";
            std::cout << "Car ID: ";
            int car_id, year, price_usd;
            std::string brand, model;
            std::cin >> car_id;
            std::cin.ignore();
            std::cout << "New Brand: ";
            std::getline(std::cin, brand);
            std::cout << "New Model: ";
            std::getline(std::cin, model);
            std::cout << "New Year: ";
            std::cin >> year;
            std::cout << "New Price (USD): ";
            std::cin >> price_usd;
            std::cin.ignore();

            response = update_admin_car(car_id, brand, model, year, price_usd, host, port);
            display_response(response);
            break;
        }

        case 4: { // Delete car
            std::cout << "\nDeleting car\n";
            std::cout << "Car ID to delete: ";
            int car_id;
            std::cin >> car_id;
            std::cin.ignore();

            response = delete_admin_car(car_id, host, port);
            display_response(response);
            break;
        }

        case 5: // View all cities
            std::cout << "\nFetching all cities...\n";
            response = fetch_admin_cities(host, port);
            display_response(response);
            break;

        case 6: { // Add new city
            std::cout << "\nAdding new city\n";
            std::cout << "City name: ";
            std::string name;
            std::getline(std::cin, name);
            std::cout << "Delivery days: ";
            int delivery_days, delivery_cost;
            std::cin >> delivery_days;
            std::cout << "Delivery cost: ";
            std::cin >> delivery_cost;
            std::cin.ignore();

            response = add_admin_city(name, delivery_days, delivery_cost, host, port);
            display_response(response);
            break;
        }

        case 7: { // Update city
            std::cout << "\nUpdating city\n";
            std::cout << "City ID: ";
            int city_id, delivery_days, delivery_cost;
            std::string name;
            std::cin >> city_id;
            std::cin.ignore();
            std::cout << "New name: ";
            std::getline(std::cin, name);
            std::cout << "New delivery days: ";
            std::cin >> delivery_days;
            std::cout << "New delivery cost: ";
            std::cin >> delivery_cost;
            std::cin.ignore();

            response = update_admin_city(city_id, name, delivery_days, delivery_cost, host, port);
            display_response(response);
            break;
        }

        case 8: { // Delete city
            std::cout << "\nDeleting city\n";
            std::cout << "City ID to delete: ";
            int city_id;
            std::cin >> city_id;
            std::cin.ignore();

            response = delete_admin_city(city_id, host, port);
            display_response(response);
            break;
        }

        case 9: // View all documents
            std::cout << "\nFetching all documents...\n";
            response = fetch_admin_documents(host, port);
            display_response(response);
            break;

        case 10: { // Add new document
            std::cout << "\nAdding new document\n";
            std::cout << "Category (purchase/registration): ";
            std::string category, name;
            std::getline(std::cin, category);
            std::cout << "Document name: ";
            std::getline(std::cin, name);

            response = add_admin_document(category, name, host, port);
            display_response(response);
            break;
        }

        case 11: { // Delete document
            std::cout << "\nDeleting document\n";
            std::cout << "Category (purchase/registration): ";
            std::string category, name;
            std::getline(std::cin, category);
            std::cout << "Document name: ";
            std::getline(std::cin, name);

            response = delete_admin_document(category, name, host, port);
            display_response(response);
            break;
        }

        case 0:
            std::cout << "\nExiting admin menu.\n";
            break;

        default:
            std::cout << "Invalid option. Please try again.\n";
    }
}