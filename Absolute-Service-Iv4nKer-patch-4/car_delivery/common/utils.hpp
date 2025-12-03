#pragma once
#include <string>

// Чтение файла
std::string read_file(const std::string& path);

// Отправка HTTP-запроса с обработкой ошибок
std::string send_http_request(const std::string& host, int port, const std::string& request);

// Извлечение тела JSON из HTTP-ответа
std::string extract_json_from_response(const std::string& http_response);