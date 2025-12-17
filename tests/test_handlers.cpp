#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>

// Заголовки проекта
#include "../common/json.hpp"
#include "../server/handlers.hpp"

using json = nlohmann::json;

// ==================== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ====================

class HandlersTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаем временную директорию data
        system("mkdir -p data");
        createTestFiles();
    }

    void TearDown() override {
        // Удаляем временные файлы
        system("rm -rf data");
    }

    void createTestFiles() {
        // Создаем тестовый cars.json
        std::ofstream cars_file("data/cars.json");
        cars_file << R"([
            {
                "id": 1,
                "brand": "Toyota",
                "model": "Camry",
                "year": 2020,
                "engine_volume": 2.5,
                "horsepower": 206,
                "steering_wheel": "right",
                "fuel_type": "petrol",
                "price_usd": 25000,
                "country": "Japan"
            },
            {
                "id": 2,
                "brand": "BMW",
                "model": "3-Series",
                "year": 2016,
                "engine_volume": 2.0,
                "horsepower": 184,
                "steering_wheel": "left",
                "fuel_type": "petrol",
                "price_usd": 17000,
                "country": "Germany"
            },
            {
                "id": 3,
                "brand": "Honda",
                "model": "Accord",
                "year": 2019,
                "engine_volume": 2.0,
                "horsepower": 158,
                "steering_wheel": "right",
                "fuel_type": "petrol",
                "price_usd": 15000,
                "country": "Japan"
            }
        ])";
        cars_file.close();

        // Создаем тестовый cities.json
        std::ofstream cities_file("data/cities.json");
        cities_file << R"([
            {
                "id": 1,
                "name": "Москва",
                "delivery_days": 30,
                "delivery_cost": 1000
            },
            {
                "id": 2,
                "name": "Санкт-Петербург",
                "delivery_days": 35,
                "delivery_cost": 1200
            }
        ])";
        cities_file.close();

        // Создаем тестовый documents.json
        std::ofstream documents_file("data/documents.json");
        documents_file << R"({
            "documents": [
                {
                    "id": 1,
                    "category": "purchase",
                    "name": "Паспорт"
                },
                {
                    "id": 2,
                    "category": "registration",
                    "name": "Заявление"
                }
            ]
        })";
        documents_file.close();
    }

    json parseResponse(const std::string& response) {
        try {
            return json::parse(response);
        } catch (...) {
            return json::object();
        }
    }
};

// БАЗОВЫЕ ТЕСТЫ И ТЕСТЫ КЛИЕНТА

TEST_F(HandlersTest, HandleGetCarsReturnsArray) {
    std::string response = handle_get_cars();
    json result = parseResponse(response);
    EXPECT_TRUE(result.is_array());
    EXPECT_GE(result.size(), 3);
}

TEST_F(HandlersTest, HandleGetCitiesReturnsArray) {
    std::string response = handle_get_cities();
    json result = parseResponse(response);
    EXPECT_TRUE(result.is_array());
    EXPECT_GE(result.size(), 2);
}

TEST_F(HandlersTest, HandleGetDocumentsReturnsDocuments) {
    std::string response = handle_get_documents();
    json result = parseResponse(response);
    EXPECT_TRUE(result.contains("documents"));
    EXPECT_TRUE(result["documents"].is_array());
    EXPECT_GE(result["documents"].size(), 2);
}

TEST_F(HandlersTest, HandlePostAdminLoginValidCredentials) {
    json creds = {{"username", "admin"}, {"password", "123"}};
    std::string response = handle_post_admin_login(creds.dump());
    json result = parseResponse(response);
    EXPECT_EQ(result["status"], "success");
    EXPECT_EQ(result["user"]["username"], "admin");
    EXPECT_EQ(result["user"]["role"], "admin");
}

TEST_F(HandlersTest, HandlePostAdminLoginInvalidCredentials) {
    json creds = {{"username", "admin"}, {"password", "wrong"}};
    std::string response = handle_post_admin_login(creds.dump());
    json result = parseResponse(response);
    EXPECT_TRUE(result.contains("error"));
}

TEST_F(HandlersTest, HandleGetSearchFindsCars) {
    std::string query = "brand=Honda&model=Accord";
    std::string response = handle_get_search(query);
    json result = parseResponse(response);
    EXPECT_TRUE(result.contains("found"));
    EXPECT_TRUE(result.contains("results"));
}

TEST_F(HandlersTest, HandlePostSearchWithSimpleFilters) {
    json request = {
        {"filters", {
            {"brand", "Toyota"},
            {"year", 2020}
        }}
    };
    
    std::string response = handle_post_search(request.dump());
    json result = parseResponse(response);
    EXPECT_TRUE(result.contains("found"));
    EXPECT_TRUE(result.contains("results"));
}

TEST_F(HandlersTest, HandlePostCalculateDelivery) {
    json request = {{"car_id", 1}, {"city_id", 1}};
    std::string response = handle_post_calculate_delivery(request.dump());
    json result = parseResponse(response);
    
    EXPECT_FALSE(result.contains("error"));
    EXPECT_TRUE(result.contains("car"));
    EXPECT_TRUE(result.contains("city"));
    EXPECT_TRUE(result.contains("calculation"));
    EXPECT_TRUE(result.contains("summary"));
    
    if (result.contains("summary")) {
        EXPECT_GT(result["summary"]["total_cost_rub"], 0);
        EXPECT_GT(result["summary"]["total_cost_usd"], 0);
    }
}

// ТЕСТЫ ДЛЯ АДМИНСКИХ ФУНКЦИЙ

TEST_F(HandlersTest, HandlePostAdminCarsValid) {
    json new_car = {
        {"brand", "TestBrand"},
        {"model", "TestModel"},
        {"year", 2024},
        {"price_usd", 30000}
    };
    
    std::string response = handle_post_admin_cars(new_car.dump());
    json result = parseResponse(response);
    
    EXPECT_EQ(result["status"], "success");
    EXPECT_EQ(result["message"], "Car added successfully");
    EXPECT_TRUE(result.contains("car"));
    EXPECT_GE(result["car"]["id"], 4); // Новый ID
}

TEST_F(HandlersTest, HandlePostAdminCitiesValid) {
    json new_city = {
        {"name", "Новосибирск"},
        {"delivery_days", 45},
        {"delivery_cost", 1500}
    };
    
    std::string response = handle_post_admin_cities(new_city.dump());
    json result = parseResponse(response);
    
    EXPECT_EQ(result["status"], "success");
    EXPECT_EQ(result["message"], "City added successfully");
    EXPECT_TRUE(result.contains("city"));
}

TEST_F(HandlersTest, HandlePostAdminDocumentsValid) {
    json new_doc = {
        {"category", "test"},
        {"name", "Тестовый документ"}
    };
    
    std::string response = handle_post_admin_documents(new_doc.dump());
    json result = parseResponse(response);
    
    EXPECT_EQ(result["status"], "success");
    EXPECT_EQ(result["message"], "Document added successfully");
    EXPECT_TRUE(result.contains("document"));
}

// ==================== ГЛАВНАЯ ФУНКЦИЯ ====================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
