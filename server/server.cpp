#include "server.hpp"
#include "../common/logger.hpp"
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
       Logger::log_info("Server started on port 8080");
    std::cout << "Сервер запущен на порту 8080\n";
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
        Logger::log_info("New connection from IP: " + client_ip);

        // Читаем весь запрос
        boost::asio::streambuf buffer;
        boost::system::error_code ec;

        // Читаем заголовки
        boost::asio::read_until(*socket, buffer, "\r\n\r\n", ec);
        if (ec && ec != boost::asio::error::eof) {
            std::cerr << "Ошибка чтения заголовков: " << ec.message() << std::endl;
              Logger::log_error("Error reading headers from " + client_ip + ": " + ec.message());
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
                  Logger::log_error("Error parsing Content-Length from " + client_ip + ": " + std::string(e.what()));
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

        // Отладочный вывод
        std::cout << "=== НЕОБРАБОТАННЫЙ ЗАПРОС ===" << std::endl;
        std::cout << request << std::endl;
        std::cout << "=== ЗАВЕРШАЮЩИЙ ЗАПРОС ===" << std::endl;

        if (request.find("GET /cars") == 0) {
                Logger::log_info("Processing GET /cars request from " + client_ip);
            response_body = handle_get_cars();
        }
        else if (request.find("POST /search") == 0) {
              Logger::log_info("Processing POST /search request from " + client_ip);
            size_t body_start = request.find("\r\n\r\n");
            if (body_start != std::string::npos) {
                std::string body = request.substr(body_start + 4);
                response_body = handle_post_search(body);
            }
            else {
                response_body = R"({"error": "No body in POST /search"})";
            }
        }
        else if (request.find("GET /search?") == 0) {
            size_t s = request.find('?'), e = request.find(' ', s);
            response_body = (s != std::string::npos && e != std::string::npos)
                ? handle_get_search(request.substr(s + 1, e - s - 1))
                : R"({"error": "Invalid query in GET /search"})";
        }
        else if (request.find("GET /cities") == 0) {
            Logger::log_info("Processing GET /cities request from " + client_ip);
            response_body = handle_get_cities();
        }
        else if (request.find("GET /documents") == 0) {
            Logger::log_info("Processing GET /documents request from " + client_ip);
            response_body = handle_get_documents();
        }
        else if (request.find("GET /delivery") == 0) {
            Logger::log_info("Processing GET /delivery request from " + client_ip);
            response_body = handle_get_delivery();
        }
        else if (request.find("POST /admin/login") == 0) {
            Logger::log_info("Processing POST /admin/login request from " + client_ip);
            size_t body_start = request.find("\r\n\r\n");
            if (body_start != std::string::npos) {
                std::string body = request.substr(body_start + 4);
                response_body = handle_post_admin_login(body);
            }
            else {
                response_body = R"({"error": "No body in POST /admin/login"})";
            }
        }
        else if (request.find("POST /calculate-delivery") == 0) {
            size_t body_start = request.find("\r\n\r\n");
            if (body_start != std::string::npos) {
                std::string body = request.substr(body_start + 4);
                response_body = handle_post_calculate_delivery(body);
            }
            else {
                response_body = R"({"error": "No body in POST /calculate-delivery"})";
            }
        }
 else if (request.find("POST /admin/cars") == 0) {
            size_t body_start = request.find("\r\n\r\n");
            if (body_start != std::string::npos) {
                std::string body = request.substr(body_start + 4);
                response_body = handle_post_admin_cars(body);
            }
            else {
                response_body = R"({"error": "No body in POST /admin/cars"})";
            }
        }
        else if (request.find("GET /admin/cars") == 0) {
            response_body = handle_get_admin_cars();
        }
        else if (request.find("PUT /admin/cars/") == 0) {
            // Извлекаем ID из URL: PUT /admin/cars/123
            size_t start = request.find("/admin/cars/") + 12;  // длина "/admin/cars/"
            size_t end = request.find(" ", start);
            if (start < end && end != std::string::npos) {
                std::string id_str = request.substr(start, end - start);
                int car_id = std::stoi(id_str);

                size_t body_start = request.find("\r\n\r\n");
                if (body_start != std::string::npos) {
                    std::string body = request.substr(body_start + 4);
                    response_body = handle_put_admin_cars(car_id, body);
                }
                else {
                    response_body = R"({"error": "No body in PUT /admin/cars/"})" + id_str + "\"}";
                }
            }
            else {
                response_body = R"({"error": "Invalid car ID in PUT /admin/cars"})";
            }
        }
        else if (request.find("DELETE /admin/cars/") == 0) {
            // Извлекаем ID из URL: DELETE /admin/cars/123
            size_t start = request.find("/admin/cars/") + 12;  // длина "/admin/cars/"
            size_t end = request.find(" ", start);
            if (start < end && end != std::string::npos) {
                std::string id_str = request.substr(start, end - start);
                int car_id = std::stoi(id_str);
                response_body = handle_delete_admin_cars(car_id);
            }
            else {
                response_body = R"({"error": "Invalid car ID in DELETE /admin/cars"})";
            }
        }
        else if (request.find("POST /admin/cities") == 0) {
            size_t body_start = request.find("\r\n\r\n");
            if (body_start != std::string::npos) {
                std::string body = request.substr(body_start + 4);
                response_body = handle_post_admin_cities(body);
            }
            else {
                response_body = R"({"error": "No body in POST /admin/cities"})";
            }
        }
        else if (request.find("GET /admin/cities") == 0) {
            response_body = handle_get_admin_cities();
        }
        else if (request.find("PUT /admin/cities/") == 0) {
            // Извлекаем ID из URL: PUT /admin/cities/123
            size_t start = request.find("/admin/cities/") + 15;  // длина "/admin/cities/"
            size_t end = request.find(" ", start);
            if (start < end && end != std::string::npos) {
                std::string id_str = request.substr(start, end - start);
                int city_id = std::stoi(id_str);

                size_t body_start = request.find("\r\n\r\n");
                if (body_start != std::string::npos) {
                    std::string body = request.substr(body_start + 4);
                    response_body = handle_put_admin_cities(city_id, body);
                }
                else {
                    response_body = R"({"error": "No body in PUT /admin/cities/"})" + id_str + "\"}";
                }
            }
            else {
                response_body = R"({"error": "Invalid city ID in PUT /admin/cities"})";
            }
        }
        else if (request.find("DELETE /admin/cities/") == 0) {
            // Извлекаем ID из URL: DELETE /admin/cities/123
            size_t start = request.find("/admin/cities/") + 15;  // длина "/admin/cities/"
            size_t end = request.find(" ", start);
            if (start < end && end != std::string::npos) {
                std::string id_str = request.substr(start, end - start);
                int city_id = std::stoi(id_str);
                response_body = handle_delete_admin_cities(city_id);
            }
            else {
                response_body = R"({"error": "Invalid city ID in DELETE /admin/cities"})";
            }
        }
        else if (request.find("POST /admin/documents") == 0) {
            size_t body_start = request.find("\r\n\r\n");
            if (body_start != std::string::npos) {
                std::string body = request.substr(body_start + 4);
                response_body = handle_post_admin_documents(body);
            }
            else {
                response_body = R"({"error": "No body in POST /admin/documents"})";
            }
        }
        else if (request.find("GET /admin/documents") == 0) {
            response_body = handle_get_admin_documents();
        }
        else if (request.find("DELETE /admin/documents") == 0) {
            size_t body_start = request.find("\r\n\r\n");
            if (body_start != std::string::npos) {
                std::string body = request.substr(body_start + 4);
                response_body = handle_delete_admin_documents(body);
            }
            else {
                response_body = R"({"error": "No body in DELETE /admin/documents"})";
            }
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
            Logger::log_info("Request from " + client_ip + " processed successfully");

    }
    catch (std::exception& e) {
        std::cerr << "[!] Ошибка обработки клиента " << e.what() << std::endl;
         Logger::log_error("Client processing error: " + std::string(e.what()));
    }
}
