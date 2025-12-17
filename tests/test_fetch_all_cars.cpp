#include <gtest/gtest.h>
#include <string>
#include "client.hpp"

// --- MOCK ---
std::string last_request;

std::string send_http_request(const std::string& host, int port, const std::string& request) {
    last_request = request;
    return R"([{"id":1,"brand":"Toyota","model":"Corolla","year":2020,"price_usd":18000}])";
}

std::string extract_json_from_response(const std::string& response) {
    return response;
}

// --- TEST ---
TEST(FetchCarsTest, FetchAllCarsFromDatabase) {
    std::string host = "127.0.0.1";
    int port = 8080;

    std::string response = fetch_all_cars(host, port);

    ASSERT_NE(last_request.find("GET /cars HTTP/1.1"), std::string::npos);

    ASSERT_NE(response.find("Toyota"), std::string::npos);
    ASSERT_NE(response.find("Corolla"), std::string::npos);
}

