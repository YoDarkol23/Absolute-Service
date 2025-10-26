// client.cpp (исправленная версия)
#include "client.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <sstream>

std::string fetch_all_cars(const std::string& host, int port) {
    try {
        boost::asio::io_context io_context;
        
        // Подключаемся к серверу
        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::ip::tcp::resolver resolver(io_context);
        
        auto endpoints = resolver.resolve(host, std::to_string(port));
        
        std::cout << "Попытка подключения к " << host << ":" << port << std::endl;
        
        boost::asio::connect(socket, endpoints);
        
        std::cout << "Подключение установлено!" << std::endl;
        
        // Формируем HTTP-запрос
        std::string request = 
            "GET /cars HTTP/1.1\r\n"
            "Host: " + host + ":" + std::to_string(port) + "\r\n"
            "Connection: close\r\n"
            "\r\n";

        // Отправляем запрос
        boost::asio::write(socket, boost::asio::buffer(request));
        std::cout << "Запрос отправлен" << std::endl;
        
        // Читаем ответ
        boost::asio::streambuf response;
        boost::system::error_code error;
        
        // Читаем весь ответ
        while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error)) {
            // Читаем все данные
        }
        
        if (error != boost::asio::error::eof) {
            throw boost::system::system_error(error);
        }
        
        // Преобразуем в строку
        std::string response_str = boost::asio::buffer_cast<const char*>(response.data());
        
        // Находим начало тела ответа
        size_t body_pos = response_str.find("\r\n\r\n");
        if (body_pos != std::string::npos) {
            return response_str.substr(body_pos + 4);
        }
        
        return response_str;
        
    } catch (std::exception& e) {
        return R"({"error": "Connection failed: )" + std::string(e.what()) + R"("})";
    }
}

std::string send_admin_request(const std::string& host, int port, const std::string& json_body) {
    return R"({"error": "Admin requests not implemented yet"})";
}