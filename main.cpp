#include <iostream>
#include <string>
#include <fstream>
#include "database.h"   // Заголовок с БД машин и стран

// ------------------------------------------------------
// ПРОВЕРКА ЛОГИНА И ПАРОЛЯ АДМИНА ЧЕРЕЗ ФАЙЛ admins.txt
// ------------------------------------------------------
// Формат файла data/admins.txt:
// username password role
// Пример строки:
// admin 123 admin
//
// Функция возвращает true, если найден админ с такими
// логином и паролем. Роль записывает в out_role.
bool check_admin_credentials(const std::string& username,
    const std::string& password,
    std::string& out_role)
{
    std::ifstream file("data/admins.txt");
    if (!file.is_open()) {
        std::cout << "Не удалось открыть файл data/admins.txt\n";
        return false;
    }

    std::string file_username;
    std::string file_password;
    std::string file_role;

    while (file >> file_username >> file_password >> file_role) {
        if (file_username == username && file_password == password) {
            out_role = file_role;
            return true;
        }
    }

    return false;
}

// ------------------------------------------------------
// ВХОД АДМИНА
// ------------------------------------------------------
// Запрашивает у пользователя логин и пароль,
// вызывает check_admin_credentials и сообщает результат.
// Возвращает true при успешном входе.
bool admin_login()
{
    std::string username;
    std::string password;
    std::string role;

    std::cout << "Введите логин администратора: ";
    std::cin >> username;

    std::cout << "Введите пароль администратора: ";
    std::cin >> password;

    bool ok = check_admin_credentials(username, password, role);
    if (ok) {
        std::cout << "Вход выполнен успешно. Роль: " << role << "\n";
        return true;
    }
    else {
        std::cout << "Неверный логин или пароль.\n";
        return false;
    }
}

// ------------------------------------------------------
// МЕНЮ РАБОТЫ С БД АВТО (ТОЛЬКО ДЛЯ АДМИНА)
// ------------------------------------------------------
// Админ может:
// 1) посмотреть все авто
// 2) добавить авто
// 3) удалить авто
// 4) изменить авто
void cars_menu()
{
    int choice = -1;
    while (choice != 0) {
        std::cout << "\n=== МЕНЮ РАБОТЫ С БД АВТО ===\n";
        std::cout << "1 - Показать все авто\n";
        std::cout << "2 - Добавить авто\n";
        std::cout << "3 - Удалить авто\n";
        std::cout << "4 - Изменить авто\n";
        std::cout << "0 - Назад\n";
        std::cout << "Выберите действие: ";
        std::cin >> choice;

        if (!std::cin) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            std::cout << "Ошибка ввода.\n";
            continue;
        }

        if (choice == 1) {
            // Просмотр всех машин
            print_cars();
        }
        else if (choice == 2) {
            // Добавление машины
            std::string brand;
            std::string model;
            std::string country;
            int year;
            double price;

            std::cout << "Введите марку: ";
            std::cin >> brand;
            std::cout << "Введите модель: ";
            std::cin >> model;
            std::cout << "Введите год: ";
            std::cin >> year;
            std::cout << "Введите цену: ";
            std::cin >> price;
            std::cout << "Введите страну: ";
            std::cin >> country;

            bool added = add_car(brand, model, year, price, country);
            if (added) {
                std::cout << "Авто успешно добавлено.\n";
            }
            else {
                std::cout << "Не удалось добавить авто.\n";
            }
        }
        else if (choice == 3) {
            // Удаление машины
            int id;
            std::cout << "Введите ID авто для удаления: ";
            std::cin >> id;

            bool deleted = delete_car(id);
            if (deleted) {
                std::cout << "Авто с ID " << id << " удалено.\n";
            }
            else {
                std::cout << "Авто с таким ID не найдено.\n";
            }
        }
        else if (choice == 4) {
            // Редактирование машины
            int id;
            std::string brand;
            std::string model;
            std::string country;
            int year;
            double price;

            std::cout << "Введите ID авто для изменения: ";
            std::cin >> id;

            int index = find_car_by_id(id);
            if (index == -1) {
                std::cout << "Авто с таким ID не найдено.\n";
                continue;
            }

            std::cout << "Введите новую марку: ";
            std::cin >> brand;
            std::cout << "Введите новую модель: ";
            std::cin >> model;
            std::cout << "Введите новый год: ";
            std::cin >> year;
            std::cout << "Введите новую цену: ";
            std::cin >> price;
            std::cout << "Введите новую страну: ";
            std::cin >> country;

            bool edited = edit_car(id, brand, model, year, price, country);
            if (edited) {
                std::cout << "Авто успешно изменено.\n";
            }
            else {
                std::cout << "Ошибка при изменении авто.\n";
            }
        }
        else if (choice == 0) {
            std::cout << "Возврат в главное админ-меню.\n";
        }
        else {
            std::cout << "Неизвестный пункт меню.\n";
        }
    }
}

// ------------------------------------------------------
// МЕНЮ РАБОТЫ С БД СТРАН (ТОЛЬКО ДЛЯ АДМИНА)
// ------------------------------------------------------
// Админ может:
// 1) посмотреть все страны
// 2) добавить страну
// 3) удалить страну
// 4) изменить страну
void countries_menu()
{
    int choice = -1;
    while (choice != 0) {
        std::cout << "\n=== МЕНЮ РАБОТЫ С БД СТРАН ===\n";
        std::cout << "1 - Показать все страны\n";
        std::cout << "2 - Добавить страну\n";
        std::cout << "3 - Удалить страну\n";
        std::cout << "4 - Изменить страну\n";
        std::cout << "0 - Назад\n";
        std::cout << "Выберите действие: ";
        std::cin >> choice;

        if (!std::cin) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            std::cout << "Ошибка ввода.\n";
            continue;
        }

        if (choice == 1) {
            // Просмотр всех стран
            print_countries();
        }
        else if (choice == 2) {
            // Добавление страны
            std::string name;
            std::string continent;

            std::cout << "Введите название страны: ";
            std::cin >> name;
            std::cout << "Введите континент: ";
            std::cin >> continent;

            bool added = add_country(name, continent);
            if (added) {
                std::cout << "Страна успешно добавлена.\n";
            }
            else {
                std::cout << "Не удалось добавить страну.\n";
            }
        }
        else if (choice == 3) {
            // Удаление страны
            int id;
            std::cout << "Введите ID страны для удаления: ";
            std::cin >> id;

            bool deleted = delete_country(id);
            if (deleted) {
                std::cout << "Страна с ID " << id << " удалена.\n";
            }
            else {
                std::cout << "Страна с таким ID не найдена.\n";
            }
        }
        else if (choice == 4) {
            // Редактирование страны
            int id;
            std::string name;
            std::string continent;

            std::cout << "Введите ID страны для изменения: ";
            std::cin >> id;

            int index = find_country_by_id(id);
            if (index == -1) {
                std::cout << "Страна с таким ID не найдена.\n";
                continue;
            }

            std::cout << "Введите новое название страны: ";
            std::cin >> name;
            std::cout << "Введите новый континент: ";
            std::cin >> continent;

            bool edited = edit_country(id, name, continent);
            if (edited) {
                std::cout << "Страна успешно изменена.\n";
            }
            else {
                std::cout << "Ошибка при изменении страны.\n";
            }
        }
        else if (choice == 0) {
            std::cout << "Возврат в главное админ-меню.\n";
        }
        else {
            std::cout << "Неизвестный пункт меню.\n";
        }
    }
}

// ------------------------------------------------------
// ГЛАВНОЕ МЕНЮ АДМИНА
// ------------------------------------------------------
// Вызывается только после успешного admin_login().
// Отсюда можно перейти в редактирование БД авто и стран.
void admin_menu()
{
    int choice = -1;
    while (choice != 0) {
        std::cout << "\n=== ГЛАВНОЕ МЕНЮ АДМИНИСТРАТОРА ===\n";
        std::cout << "1 - Изменить БД автомобилей\n";
        std::cout << "2 - Изменить БД стран\n";
        std::cout << "0 - Выход из админ-меню\n";
        std::cout << "Выберите действие: ";
        std::cin >> choice;

        if (!std::cin) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            std::cout << "Ошибка ввода.\n";
            continue;
        }

        if (choice == 1) {
            cars_menu();
        }
        else if (choice == 2) {
            countries_menu();
        }
        else if (choice == 0) {
            std::cout << "Выход из админ-меню.\n";
        }
        else {
            std::cout << "Неизвестный пункт меню.\n";
        }
    }
}

// ------------------------------------------------------
// ГЛАВНОЕ МЕНЮ ПРОГРАММЫ
// ------------------------------------------------------
// Клиент может только смотреть списки машин и стран.
// Пункт 7 — вход в админку, где разрешено редактирование.
int main()
{
    setlocale(LC_ALL, "Russian");

    int choice = -1;
    bool running = true;

    while (running) {
        std::cout << "\n=== ГЛАВНОЕ МЕНЮ ПРОГРАММЫ ===\n";
        std::cout << "1 - Показать список автомобилей (клиент)\n";
        std::cout << "2 - Показать список стран (клиент)\n";
        std::cout << "7 - Вход администратора\n";
        std::cout << "0 - Выход\n";
        std::cout << "Выберите действие: ";
        std::cin >> choice;

        if (!std::cin) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            std::cout << "Ошибка ввода.\n";
            continue;
        }

        if (choice == 1) {
            print_cars();
        }
        else if (choice == 2) {
            print_countries();
        }
        else if (choice == 7) {
            bool logged_in = admin_login();
            if (logged_in) {
                admin_menu();   // Только здесь вход в редактирование БД
            }
            else {
                std::cout << "Доступ к админ-меню запрещён.\n";
            }
        }
        else if (choice == 0) {
            std::cout << "Выход из программы.\n";
            running = false;
        }
        else {
            std::cout << "Неизвестный пункт меню.\n";
        }
    }

    return 0;
}
