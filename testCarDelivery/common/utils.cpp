#include "utils.hpp"
#include <fstream>
#include <sstream>
#include <boost/asio.hpp>

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

// Реализация отправки HTTP-запроса
std::string send_http_request(const std::string& host, int port, const std::string& request) {
    using boost::asio::ip::tcp;
    boost::asio::io_context io_context;

    // Находим IP по имени хоста
    tcp::resolver resolver(io_context);
    tcp::socket socket(io_context);
    auto endpoint = resolver.resolve(host, std::to_string(port));
    boost::asio::connect(socket, endpoint);

    // Отправляем запрос
    boost::asio::write(socket, boost::asio::buffer(request));

    // Читаем ответ (заголовки + тело)
    boost::asio::streambuf response_buffer;
    boost::system::error_code ec;

    // Читаем до конца (для маленьких ответов — нормально)
    while (boost::asio::read(socket, response_buffer, boost::asio::transfer_at_least(1), ec));
    
    return std::string(
        std::istreambuf_iterator<char>(&response_buffer),
        std::istreambuf_iterator<char>()
    );
}