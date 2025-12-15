#include "utils.hpp"
#include <fstream>
#include <sstream>
#include <boost/asio.hpp>
#include <iostream>

std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return R"([{"error": "File not found"})";
    }
    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

std::string extract_json_from_response(const std::string& http_response) {
    size_t body_start = http_response.find("\r\n\r\n");
    if (body_start != std::string::npos) {
        return http_response.substr(body_start + 4);
    }
    body_start = http_response.find("\n\n");
    if (body_start != std::string::npos) {
        return http_response.substr(body_start + 2);
    }
    return http_response;
}

std::string send_http_request(const std::string& host, int port, const std::string& request) {
    try {
        using boost::asio::ip::tcp;
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::socket socket(io_context);
        auto endpoint = resolver.resolve(host, std::to_string(port));
        boost::asio::connect(socket, endpoint);
        boost::asio::write(socket, boost::asio::buffer(request));

        boost::asio::streambuf response_buffer;
        boost::system::error_code ec;
        while (boost::asio::read(socket, response_buffer, boost::asio::transfer_at_least(1), ec));
        
        return std::string(
            std::istreambuf_iterator<char>(&response_buffer),
            std::istreambuf_iterator<char>()
        );
    } catch (const std::exception& e) {
        // Возвращаем JSON с ошибкой вместо падения
        return R"({"error": "Connection failed", "message": ")" + std::string(e.what()) + "\"}";
    }
}