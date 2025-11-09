#pragma once  // Защита от двойного включения

#include <string>

/**
 * Читает весь файл и возвращает его содержимое как строку.
 * Используется сервером для загрузки cars.json.
 * 
 * @param path — путь к файлу (например, "data/cars.json")
 * @return содержимое файла или строку с ошибкой, если файл не найден
 */
std::string read_file(const std::string& path);

/**
 * Отправляет HTTP-запрос на указанный хост и порт.
 * Используется клиентом для связи с сервером.
 * 
 * @param host — IP или домен (например, "127.0.0.1")
 * @param port — порт сервера (8080 для клиентов, 8081 для админа)
 * @param request — полный HTTP-запрос (включая \r\n\r\n)
 * @return ответ сервера (заголовки + тело)
 */
std::string send_http_request(const std::string& host, int port, const std::string& request);