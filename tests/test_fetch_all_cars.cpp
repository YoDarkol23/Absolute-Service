#include <gtest/gtest.h>
#include <string>
#include "client.hpp"

// --- MOCK ---
// сохраняем последний запрос
std::string last_request;

// мок функции, которая делает HTTP-запрос
std::string send_http_request(const std::string& host, int port, const std::string& request) {
    last_request = request;
    return R"([{"id":1,"brand":"Toyota","model":"Corolla","year":2020,"price_usd":18000}])";
}

// мок функции, которая извлекает JSON из HTTP-ответа
std::string extract_json_from_response(const std::string& response) {
    return response;  // просто возвращаем ответ
}

// --- TEST ---
TEST(FetchCarsTest, FetchAllCarsFromDatabase) {
    std::string host = "127.0.0.1";
    int port = 8080;

    std::string response = fetch_all_cars(host, port);

    // проверка правильности запроса
    ASSERT_NE(last_request.find("GET /cars HTTP/1.1"), std::string::npos);

    // проверка данных
    ASSERT_NE(response.find("Toyota"), std::string::npos);
    ASSERT_NE(response.find("Corolla"), std::string::npos);
}
