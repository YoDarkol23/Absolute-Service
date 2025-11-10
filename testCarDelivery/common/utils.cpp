#include "utils.hpp"
#include <fstream>
#include <sstream>
#include <boost/asio.hpp>
#include <iostream>

// Реализация чтения файла
std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        // Если файл не найден — возвращаем JSON с ошибкой
        return R"([{"error": "File not found: )" + path + "\"}]";
    }
    // Читаем всё содержимое файла в строку
    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

// Реализация отправки HTTP-запроса (ИСПРАВЛЕННАЯ)
std::string send_http_request(const std::string& host, int port, const std::string& request) {
    try {
        using boost::asio::ip::tcp;
        boost::asio::io_context io_context;

        // Находим IP по имени хоста
        tcp::resolver resolver(io_context);
        tcp::socket socket(io_context);
        auto endpoints = resolver.resolve(host, std::to_string(port));
        boost::asio::connect(socket, endpoints);

        // Отправляем запрос
        boost::asio::write(socket, boost::asio::buffer(request));

        // Читаем ответ
        boost::asio::streambuf response_buffer;
        boost::system::error_code ec;
        
        // Читаем до конца соединения
        while (boost::asio::read(socket, response_buffer, 
                                boost::asio::transfer_at_least(1), ec)) {
            if (ec) break;
        }
        
        // Если ошибка - не break, а end of file - это нормально
        if (ec != boost::asio::error::eof) {
            throw boost::system::system_error(ec);
        }
        
        std::string response = boost::asio::buffer_cast<const char*>(response_buffer.data());
        return response;
        
    } catch (std::exception& e) {
        std::cerr << "❌ Ошибка HTTP-запроса: " << e.what() << std::endl;
        return "HTTP/1.1 500 Internal Server Error\r\n\r\nError: " + std::string(e.what());
    }
}