#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <iostream>

// ------------------------------------------------------
// СТРУКТУРЫ БД
// ------------------------------------------------------

struct Car {
	int id;
	std::string brand;
	std::string model;
	int year;
	double price;
	std::string country;
};

struct Country {
	int id;
	std::string name;
	std::string continent;
};

// ------------------------------------------------------
// ГЛОБАЛЬНЫЕ ВЕКТОРЫ — НАША "БАЗА ДАННЫХ"
// ------------------------------------------------------

inline std::vector<Car> g_cars = {
	{1, "Toyota",  "Camry",    2020, 25000, "Japan"},
	{2, "Kia",     "Sportage", 2021, 30000, "Korea"},
	{3, "Hyundai", "Elantra",  2019, 18000, "Korea"}
};

inline std::vector<Country> g_countries = {
	{1, "Japan",   "Asia"},
	{2, "Korea",   "Asia"},
	{3, "Germany", "Europe"},
	{4, "USA",     "North America"}
};

// ------------------------------------------------------
// ПОМОГАЮЩИЕ ФУНКЦИИ (ПОИСК ID)
// ------------------------------------------------------

inline int next_car_id() {
	int max_id = 0;
	for (auto& c : g_cars) if (c.id > max_id) max_id = c.id;
	return max_id + 1;
}

inline int next_country_id() {
	int max_id = 0;
	for (auto& c : g_countries) if (c.id > max_id) max_id = c.id;
	return max_id + 1;
}

inline int find_car_by_id(int id) {
	for (size_t i = 0; i < g_cars.size(); i++)
		if (g_cars[i].id == id) return i;
	return -1;
}

inline int find_country_by_id(int id) {
	for (size_t i = 0; i < g_countries.size(); i++)
		if (g_countries[i].id == id) return i;
	return -1;
}

// ------------------------------------------------------
// ОПЕРАЦИИ С АВТО
// ------------------------------------------------------

inline void print_cars() {
	std::cout << "\n--- БД МАШИН ---\n";
	for (auto& c : g_cars)
		std::cout << "ID: " << c.id
		<< ", " << c.brand
		<< " " << c.model
		<< ", " << c.year
		<< ", " << c.price
		<< " USD, " << c.country << '\n';
}

inline bool add_car(const std::string& brand,
	const std::string& model,
	int year,
	double price,
	const std::string& country)
{
	g_cars.push_back({
		next_car_id(),
		brand,
		model,
		year,
		price,
		country
		});
	return true;
}

inline bool delete_car(int id) {
	int index = find_car_by_id(id);
	if (index == -1) return false;
	g_cars.erase(g_cars.begin() + index);
	return true;
}

inline bool edit_car(int id,
	const std::string& brand,
	const std::string& model,
	int year,
	double price,
	const std::string& country)
{
	int index = find_car_by_id(id);
	if (index == -1) return false;

	g_cars[index].brand = brand;
	g_cars[index].model = model;
	g_cars[index].year = year;
	g_cars[index].price = price;
	g_cars[index].country = country;

	return true;
}

// ------------------------------------------------------
// ОПЕРАЦИИ С СТРАНАМИ
// ------------------------------------------------------

inline void print_countries() {
	std::cout << "\n--- БД СТРАН ---\n";
	for (auto& c : g_countries)
		std::cout << "ID: " << c.id
		<< ", " << c.name
		<< ", " << c.continent << '\n';
}

inline bool add_country(const std::string& name,
	const std::string& continent)
{
	g_countries.push_back({ next_country_id(), name, continent });
	return true;
}

inline bool delete_country(int id) {
	int index = find_country_by_id(id);
	if (index == -1) return false;
	g_countries.erase(g_countries.begin() + index);
	return true;
}

inline bool edit_country(int id,
	const std::string& new_name,
	const std::string& new_continent)
{
	int index = find_country_by_id(id);
	if (index == -1) return false;

	g_countries[index].name = new_name;
	g_countries[index].continent = new_continent;

	return true;
}

#endif // DATABASE_H