
#include "client.hpp"
#include "../common/utils.hpp"
#include "json.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

using json = nlohmann::json;

void print_car_table(const json& cars);
void print_cities_table(const json& cities);
void print_documents_list(const json& documents);
void print_delivery_process(const json& process);
void print_search_results(const json& results);
void print_admin_login_result(const json& result);

std::string fetch_all_cars(const std::string& host, int port) {
    std::string request = 
        "GET /cars HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";
    return send_http_request(host, port, request);
}

std::string fetch_cars_by_specs(const std::string& specs, const std::string& host, int port) {
    try {
        json filters = json::object();
        std::istringstream iss(specs);
        std::string pair;
        while (std::getline(iss, pair, ',')) {
            size_t pos = pair.find('=');
            if (pos != std::string::npos) {
                std::string key = pair.substr(0, pos);
                std::string value = pair.substr(pos + 1);
                filters[key] = value;
            }
        }
        json request_body = {{"filters", filters}};
        std::string body = request_body.dump();
        std::string request = 
            "POST /search HTTP/1.1\r\n"
            "Host: " + host + "\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;
        return send_http_request(host, port, request);
    } catch (...) {
        return R"({"error": "Invalid request format"})";
    }
}

std::string fetch_cars_by_brand_model(const std::string& brand, const std::string& model, const std::string& host, int port) {
    std::string query = "brand=" + brand;
    if (!model.empty()) query += "&model=" + model;
    std::string request = 
        "GET /search?" + query + " HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";
    return send_http_request(host, port, request);
}

std::string fetch_delivery_cities(const std::string& host, int port) {
    std::string request = 
        "GET /cities HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";
    return send_http_request(host, port, request);
}

std::string fetch_required_documents(const std::string& host, int port) {
    std::string request = 
        "GET /documents HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";
    return send_http_request(host, port, request);
}

std::string fetch_delivery_process(const std::string& host, int port) {
    std::string request = 
        "GET /delivery HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";
    return send_http_request(host, port, request);
}

std::string fetch_admin_login(const std::string& username, const std::string& password, const std::string& host, int port) {
    try {
        json body_json = {{"username", username}, {"password", password}};
        std::string body = body_json.dump();
        std::string request = 
            "POST /admin/login HTTP/1.1\r\n"
            "Host: " + host + "\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;
        return send_http_request(host, port, request);
    } catch (...) {
        return R"({"error": "Failed to form login request"})";
    }
}

void print_car_table(const json& cars) {
    if (cars.empty()) {
        std::cout << "No cars available.\n";
        return;
    }
    std::cout << std::setw(5) << "ID"
              << " | " << std::setw(12) << "Brand"
              << " | " << std::setw(16) << "Model"
              << " | " << std::setw(4) << "Year"
              << " | " << std::setw(8) << "Price,$"
              << " | " << std::setw(8) << "Steering" << '\n';
    std::cout << std::string(65, '-') << '\n';
    for (const auto& car : cars) {
        int id = car.value("id", 0);
        std::string brand = car.value("brand", "");
        std::string model = car.value("model", "");
        int year = car.value("year", 0);
        int price = car.value("price_usd", 0);
        std::string steering = (car.value("steering_wheel", "") == "left") ? "Left" : "Right";
        std::cout << std::setw(5) << id
                  << " | " << std::setw(12) << brand
                  << " | " << std::setw(16) << model
                  << " | " << std::setw(4) << year
                  << " | " << std::setw(8) << price
                  << " | " << std::setw(8) << steering << '\n';
    }
    std::cout << std::string(65, '-') << '\n';
}

void print_cities_table(const json& cities) {
    if (cities.empty()) {
        std::cout << "No delivery cities available.\n";
        return;
    }
    std::cout << std::setw(5) << "ID"
              << " | " << std::setw(16) << "City"
              << " | " << std::setw(8) << "Days"
              << " | " << std::setw(10) << "Cost,$" << '\n';
    std::cout << std::string(50, '-') << '\n';
    for (const auto& city : cities) {
        std::cout << std::setw(5) << city.value("id", 0)
                  << " | " << std::setw(16) << city.value("name", "")
                  << " | " << std::setw(8) << city.value("delivery_days", 0)
                  << " | " << std::setw(10) << city.value("delivery_cost", 0) << '\n';
    }
    std::cout << std::string(50, '-') << '\n';
}

void print_documents_list(const json& documents) {
    if (!documents.contains("documents") || documents["documents"].empty()) {
        std::cout << "No documents required.\n";
        return;
    }
    std::cout << "\nRequired documents for vehicle purchase and registration:\n";
    std::cout << std::string(55, '-') << '\n';
    std::string current_category;
    for (const auto& doc : documents["documents"]) {
        std::string category = doc.value("category", "");
        if (category != current_category) {
            current_category = category;
            std::cout << '\n' << (category == "purchase" ? "Purchase documents:" : "Registration documents:") << '\n';
        }
        std::cout << " - " << doc.value("name", "") << '\n';
    }
}

void print_delivery_process(const json& process) {
    if (!process.contains("process") || process["process"].empty()) {
        std::cout << "Delivery process information is unavailable.\n";
        return;
    }
    std::cout << "\nVehicle delivery process:\n";
    std::cout << std::string(40, '-') << '\n';
    if (process.contains("progress")) std::cout << "Progress: " << process.value("progress", 0) << "%\n";
    if (process.contains("duration")) std::cout << "Estimated duration: " << process.value("duration", "") << '\n';
    if (process.contains("cost")) std::cout << "Delivery cost: " << process.value("cost", "") << '\n';
    std::cout << '\n';
    for (const auto& step : process["process"]) {
        std::string status = step.value("status", "");
        std::string label = (status == "completed") ? "[completed]" :
                           (status == "in_progress") ? "[in progress]" : "[pending]";
        std::cout << "Step " << step.value("step", 0) << " " << label
                  << ": " << step.value("description", "") << '\n';
    }
}

void print_search_results(const json& results) {
    if (results.contains("error")) {
        std::cout << "Error: " << results["error"] << '\n';
        return;
    }
    if (results.contains("message")) std::cout << results["message"] << '\n';
    if (results.contains("found")) {
        int n = results["found"];
        std::cout << "Found " << n << " vehicle(s).\n";
        if (n > 0 && results.contains("results")) print_car_table(results["results"]);
        else if (n == 0) std::cout << "No vehicles match the specified criteria.\n";
    }
}

void print_admin_login_result(const json& result) {
    if (result.contains("error")) {
        std::cout << "Authentication failed: " << result["error"] << '\n';
        return;
    }
    if (result.value("status", "") == "success") {
        std::cout << "Authentication successful.\n";
        if (result.contains("user")) {
            auto& u = result["user"];
            std::cout << "User: " << u.value("username", "unknown")
                      << " (" << u.value("role", "user") << ")\n";
        }
    }
}

void display_response(const std::string& response) {
    try {
        // Извлекаем тело JSON из HTTP-ответа (общая функция из common/utils.cpp)
        std::string json_body = extract_json_from_response(response);
        auto j = json::parse(json_body);

        if (j.is_array() && !j.empty() && j[0].contains("brand")) {
            print_car_table(j);
        } else if (j.is_array() && !j.empty() && j[0].contains("name") && j[0].contains("delivery_days")) {
            print_cities_table(j);
        } else if (j.contains("documents")) {
            print_documents_list(j);
        } else if (j.contains("process")) {
            print_delivery_process(j);
        } else if (j.contains("results") || j.contains("found")) {
            print_search_results(j);
        } else if (j.contains("status") || j.contains("error")) {
            print_admin_login_result(j);
        } else {
            std::cout << j.dump(2) << '\n';
        }
    } catch (const json::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << "\nRaw response:\n" << response << '\n';
    }
}
