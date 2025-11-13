#include "utils.hpp"
#include <fstream>
#include <sstream>
#include <boost/asio.hpp>
#include <iostream>

std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "❌ Ошибка открытия файла: " << path << std::endl;
        return ""; // Возвращаем пустую строку вместо JSON с ошибкой
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string send_http_request(const std::string& host, int port, const std::string& request) {
    try {
        using boost::asio::ip::tcp;
        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        tcp::socket socket(io_context);
        
        auto endpoints = resolver.resolve(host, std::to_string(port));
        boost::asio::connect(socket, endpoints);

        // Отправляем запрос
        boost::asio::write(socket, boost::asio::buffer(request));

        // Читаем ответ
        boost::asio::streambuf response;
        boost::system::error_code ec;
        
        // Читаем все данные
        while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), ec)) {
            // Продолжаем читать
        }
        
        // Если ошибка - не EOF, то это проблема
        if (ec && ec != boost::asio::error::eof) {
            throw boost::system::system_error(ec);
        }
        
        // Преобразуем буфер в строку
        std::istream response_stream(&response);
        std::stringstream ss;
        ss << response_stream.rdbuf();
        
        return ss.str();
        
    } catch (std::exception& e) {
        std::cerr << "❌ Ошибка HTTP-запроса к " << host << ":" << port << ": " << e.what() << std::endl;
        return R"({"error": "Ошибка подключения к серверу"})";
    }
}