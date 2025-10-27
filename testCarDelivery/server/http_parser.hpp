// http_parser.hpp
#pragma once

#include <string>
#include <unordered_map>
#include <sstream>

/**
 * @brief Структура для представления HTTP-запроса
 */
struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    
    // Вспомогательные методы
    std::string get_header(const std::string& name) const;
    bool has_header(const std::string& name) const;
    bool is_json_content() const;
};

/**
 * @brief Структура для представления HTTP-ответа
 */
struct HttpResponse {
    std::string version = "HTTP/1.1";
    unsigned int status_code = 200;
    std::string status_text = "OK";
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    
    // Методы для построения ответа
    void set_content_type(const std::string& type);
    void set_json_content();
    std::string to_string() const;
    
    // Статические методы для быстрого создания ответов
    static HttpResponse json_response(const std::string& json_body, unsigned int status = 200);
    static HttpResponse error_response(unsigned int status, const std::string& message);
};

/**
 * @brief Класс для парсинга HTTP-запросов
 */
class HttpParser {
public:
    static HttpRequest parse_request(const std::string& raw_request);
    static bool parse_request_line(const std::string& line, HttpRequest& request);
    static bool parse_header_line(const std::string& line, HttpRequest& request);
    
private:
    static void trim(std::string& str);
    static std::pair<std::string, std::string> split_header(const std::string& line);
};