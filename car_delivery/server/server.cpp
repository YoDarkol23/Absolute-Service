
/**
 * @file server/server.cpp
 * @brief Реализация сервера testCarDelivery.
 * 
 * Обрабатывает все запросы на одном порту (8080).
 * Поддерживает:
 *   - GET /cars, /cities, /documents, /delivery
 *   - GET /search?brand=...&model=...
 *   - POST /search (JSON body)
 *   - POST /admin/login (аутентификация)
 * 
 * Использует пул потоков для многопоточности.
 */

#include "server.hpp"
#include "handlers.hpp"
#include <iostream>
#include <sstream>
#include <string>

// === ThreadPool ===
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
        if (worker.joinable()) worker.join();
    }
}

template void ThreadPool::enqueue<std::function<void()>>(std::function<void()>&&);

// === CarDeliveryServer ===
CarDeliveryServer::CarDeliveryServer(unsigned short port)
    : acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {}

void CarDeliveryServer::run() {
    std::cout << "🚀 Сервер запущен на порту 8080\n";
    std::cout << "Ожидание подключений...\n";

    while (true) {
        auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
        acceptor_.accept(*socket);
        client_pool_.enqueue([this, socket]() {
            handle_client(socket);
        });
    }
}

void CarDeliveryServer::handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    try {
        auto remote_ep = socket->remote_endpoint();
        std::string client_ip = remote_ep.address().to_string();
        std::cout << "[+] Новое подключение от " << client_ip << std::endl;

        // Читаем весь запрос
        boost::asio::streambuf buffer;
        boost::system::error_code ec;

        // Читаем заголовки
        boost::asio::read_until(*socket, buffer, "\r\n\r\n", ec);
        if (ec && ec != boost::asio::error::eof) {
            std::cerr << "Ошибка чтения заголовков: " << ec.message() << std::endl;
            return;
        }

        std::string request{
            std::istreambuf_iterator<char>(&buffer),
            std::istreambuf_iterator<char>()
        };

        // Определяем длину тела для POST запросов
        size_t content_length = 0;
        size_t cl_pos = request.find("Content-Length: ");
        if (cl_pos != std::string::npos) {
            size_t end_line = request.find("\r\n", cl_pos);
            std::string cl_str = request.substr(cl_pos + 16, end_line - cl_pos - 16);
            try {
                content_length = std::stoul(cl_str);
            }
            catch (const std::exception& e) {
                std::cerr << "Ошибка парсинга Content-Length: " << e.what() << std::endl;
            }
        }

        // Читаем тело если есть
        if (content_length > 0) {
            // Уже прочитали часть тела в буфер после заголовков
            size_t body_start_pos = request.find("\r\n\r\n");
            if (body_start_pos != std::string::npos) {
                body_start_pos += 4;
                size_t already_read = request.length() - body_start_pos;

                // Добираем оставшиеся данные если нужно
                if (already_read < content_length) {
                    size_t remaining = content_length - already_read;
                    std::vector<char> body_part(remaining);

                    boost::asio::read(*socket, boost::asio::buffer(body_part), ec);
                    if (!ec) {
                        request += std::string(body_part.begin(), body_part.end());
                    }
                }
            }
        }

        // Обработка запросов
        std::string response_body;

        else if (request.find("POST /search") == 0) {
            size_t body_start = request.find("\r\n\r\n");
            if (body_start != std::string::npos) {
                std::string body = request.substr(body_start + 4);
                response_body = handle_post_search(body);
            }
            else {
                response_body = R"({"error": "No body in POST /search"})";
            }
        }
        // ... остальные обработчики остаются без изменений
        else if (request.find("GET /cars") == 0) {
            response_body = handle_get_cars();
        }
        else if (request.find("GET /search?") == 0) {
            size_t s = request.find('?'), e = request.find(' ', s);
            response_body = (s != std::string::npos && e != std::string::npos)
                ? handle_get_search(request.substr(s + 1, e - s - 1))
                : R"({"error": "Invalid query in GET /search"})";
        }
        else if (request.find("GET /cities") == 0) {
            response_body = handle_get_cities();
        }
        else if (request.find("GET /documents") == 0) {
            response_body = handle_get_documents();
        }
        else if (request.find("GET /delivery") == 0) {
            response_body = handle_get_delivery();
        }
        else if (request.find("POST /admin/login") == 0) {
            size_t body_start = request.find("\r\n\r\n");
            if (body_start != std::string::npos) {
                std::string body = request.substr(body_start + 4);
                response_body = handle_post_admin_login(body);
            }
            else {
                response_body = R"({"error": "No body in POST /admin/login"})";
            }
        }
        else {
            response_body = R"({"error": "Endpoint not supported"})";
        }

        // Отправка ответа
        std::ostringstream resp;
        resp << "HTTP/1.1 200 OK\r\n"
            << "Content-Type: application/json\r\n"
            << "Content-Length: " << response_body.size() << "\r\n"
            << "Connection: close\r\n\r\n"
            << response_body;

        boost::asio::write(*socket, boost::asio::buffer(resp.str()));
        std::cout << "[✓] Запрос от " << client_ip << " обработан\n";

    }
    catch (std::exception& e) {
        std::cerr << "[!] Ошибка обработки клиента " << e.what() << std::endl;
    }
}
