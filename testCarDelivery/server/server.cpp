/**
 * @file server/server.cpp
 * @brief Реализация сервера CarDelivery.
 * 
 * Содержит:
 * - Реализацию пула потоков (ThreadPool)
 * - Логику приёма подключений на двух портах:
 *     • 8080 — обычные клиенты
 *     • 8081 — администраторы (приоритетная обработка)
 * - Обработку HTTP-запросов и маршрутизацию к обработчикам
 * 
 * ВАЖНО: Вся бизнес-логика (расчёт, фильтрация) находится в handlers.cpp.
 * Здесь только сетевая часть и многопоточность.
 */

#include "server.hpp"
#include "handlers.hpp"
#include <iostream>
#include <sstream>
#include <regex>

// === Реализация парсинга HTTP ===

bool parse_request_line(const std::string& line, HttpRequest& request) {
    std::regex request_line_regex(R"(^(\w+)\s+([^\s]+)\s+HTTP/(\d\.\d)$)");
    std::smatch matches;
    
    if (std::regex_match(line, matches, request_line_regex) && matches.size() == 4) {
        request.method = matches[1];
        request.path = matches[2];
        request.version = matches[3];
        return true;
    }
    return false;
}

void parse_headers(const std::vector<std::string>& header_lines, HttpRequest& request) {
    std::regex header_regex(R"(^([^:]+):\s*(.+)$)");
    
    for (const auto& line : header_lines) {
        std::smatch matches;
        if (std::regex_match(line, matches, header_regex) && matches.size() == 3) {
            std::string key = matches[1];
            std::string value = matches[2];
            
            // Приводим ключ к нижнему регистру для единообразия
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            request.headers[key] = value;
        }
    }
}

HttpRequest parse_http_request(const std::string& raw_request) {
    HttpRequest request;
    
    // Разделяем запрос на строки
    std::istringstream stream(raw_request);
    std::string line;
    std::vector<std::string> lines;
    
    while (std::getline(stream, line)) {
        // Убираем \r из конца строки
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
    
    if (lines.empty()) {
        throw std::runtime_error("Empty HTTP request");
    }
    
    // Парсим первую строку (метод, путь, версия)
    if (!parse_request_line(lines[0], request)) {
        throw std::runtime_error("Invalid HTTP request line: " + lines[0]);
    }
    
    // Ищем разделитель заголовков и тела
    size_t header_end = 0;
    for (size_t i = 1; i < lines.size(); ++i) {
        if (lines[i].empty()) {
            header_end = i;
            break;
        }
    }
    
    // Парсим заголовки
    if (header_end > 1) {
        std::vector<std::string> header_lines(lines.begin() + 1, lines.begin() + header_end);
        parse_headers(header_lines, request);
    }
    
    // Парсим тело (если есть)
    if (header_end > 0 && header_end + 1 < lines.size()) {
        for (size_t i = header_end + 1; i < lines.size(); ++i) {
            if (!request.body.empty()) {
                request.body += "\n";
            }
            request.body += lines[i];
        }
    }
    
    return request;
}

std::unordered_map<std::string, std::string> parse_query_params(const std::string& path) {
    std::unordered_map<std::string, std::string> params;
    
    size_t query_start = path.find('?');
    if (query_start == std::string::npos) {
        return params;
    }
    
    std::string query_string = path.substr(query_start + 1);
    std::istringstream query_stream(query_string);
    std::string pair;
    
    while (std::getline(query_stream, pair, '&')) {
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = pair.substr(0, eq_pos);
            std::string value = pair.substr(eq_pos + 1);
            params[key] = value;
        }
    }
    
    return params;
}

std::string create_http_response(const std::string& body, int status_code, 
                                const std::string& content_type) {
    std::ostringstream response;
    
    response << "HTTP/1.1 " << status_code << " ";
    switch (status_code) {
        case 200: response << "OK"; break;
        case 400: response << "Bad Request"; break;
        case 404: response << "Not Found"; break;
        case 405: response << "Method Not Allowed"; break;
        case 500: response << "Internal Server Error"; break;
        default: response << "Unknown"; break;
    }
    
    response << "\r\n"
             << "Content-Type: " << content_type << "\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Connection: close\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "\r\n"
             << body;
    
    return response.str();
}

// === Реализация пула потоков ===

ThreadPool::ThreadPool(size_t threads) : stop(false) {
    for (size_t i = 0; i < threads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    condition.wait(lock, [this] { return stop || !tasks.empty(); });
                    if (stop && tasks.empty()) return;
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        });
    }
}

template<class F>
void ThreadPool::enqueue(F&& f) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        tasks.emplace(std::forward<F>(f));
    }
    condition.notify_one();
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

template void ThreadPool::enqueue<std::function<void()>>(std::function<void()>&&);

// === Реализация сервера ===

CarDeliveryServer::CarDeliveryServer(unsigned short client_port, unsigned short admin_port)
    : client_acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), client_port)),
      admin_acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), admin_port)) {}

void CarDeliveryServer::run() {
    std::cout << "🚗 Сервер CarDelivery запущен:\n";
    std::cout << "   • Клиентский порт: " << client_acceptor_.local_endpoint().port() << "\n";
    std::cout << "   • Админский порт: " << admin_acceptor_.local_endpoint().port() << "\n";
    std::cout << "   Ожидание подключений...\n\n";
    start_acceptors();
}

void CarDeliveryServer::start_acceptors() {
    std::thread client_thread([this]() {
        while (true) {
            auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
            client_acceptor_.accept(*socket);
            client_pool_.enqueue([this, socket]() {
                handle_client(socket, false);
            });
        }
    });

    std::thread admin_thread([this]() {
        while (true) {
            auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
            admin_acceptor_.accept(*socket);
            admin_pool_.enqueue([this, socket]() {
                handle_client(socket, true);
            });
        }
    });

    client_thread.detach();
    admin_thread.detach();
    
    // Главный поток продолжает работу
    io_context_.run();
}

void CarDeliveryServer::handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket, bool is_admin) {
    try {
        auto remote_ep = socket->remote_endpoint();
        std::string client_ip = remote_ep.address().to_string();

        std::cout << "[+] Новое " << (is_admin ? "АДМИН" : "КЛИЕНТ") 
                  << "-подключение от " << client_ip << std::endl;

        // Читаем запрос
        boost::asio::streambuf buffer;
        boost::system::error_code error;
        
        // Читаем до конца заголовков
        size_t bytes_read = boost::asio::read_until(*socket, buffer, "\r\n\r\n", error);
        
        if (error && error != boost::asio::error::eof) {
            throw std::runtime_error("Ошибка чтения запроса: " + error.message());
        }

        // Преобразуем в строку
        std::string request_data{
            std::istreambuf_iterator<char>(&buffer),
            std::istreambuf_iterator<char>()
        };

        std::string response;
        
        try {
            // Парсим HTTP-запрос
            HttpRequest http_request = parse_http_request(request_data);
            
            std::cout << "[" << (is_admin ? "ADMIN" : "CLIENT") << "] "
                      << http_request.method << " " << http_request.path 
                      << " from " << client_ip << std::endl;

            // Обрабатываем запрос
            if (is_admin) {
                response = handle_admin_request(http_request);
            } else {
                response = handle_client_request(http_request);
            }
            
        } catch (const std::exception& e) {
            std::cerr << "[!] Ошибка парсинга запроса: " << e.what() << std::endl;
            std::string error_body = R"({"error": ")" + std::string(e.what()) + "\"}";
            response = create_http_response(error_body, 400);
        }

        // Отправляем ответ
        boost::asio::write(*socket, boost::asio::buffer(response));
        socket->close();
        
        std::cout << "[✓] Запрос от " << client_ip << " обработан успешно\n\n";

    } catch (std::exception& e) {
        std::cerr << "[!] Ошибка при обработке " 
                  << (is_admin ? "админ" : "клиент") 
                  << "-запроса: " << e.what() << std::endl;
    }
}