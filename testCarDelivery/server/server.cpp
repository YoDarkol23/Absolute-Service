/**
 * @file server/server.cpp
 * @brief Реализация сервера testCarDelivery.
 * 
 * Обрабатывает запросы на двух портах:
 *   - 8080 — обычные клиенты
 *   - 8081 — администраторы
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
CarDeliveryServer::CarDeliveryServer(unsigned short client_port, unsigned short admin_port)
    : client_acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), client_port))
    , admin_acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), admin_port)) {}

void CarDeliveryServer::run() {
    std::cout << "🚀 Сервер запущен!\n";
    std::cout << "   Обычные запросы: http://localhost:8080\n";
    std::cout << "   Админка:         http://localhost:8081\n";
    std::cout << "Ожидание подключений...\n";

    // Запускаем обработку клиентских подключений
    std::thread client_thread([this]() {
        while (true) {
            try {
                auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
                client_acceptor_.accept(*socket);
                client_pool_.enqueue([this, socket]() {
                    handle_client(socket);
                });
            } catch (std::exception& e) {
                std::cerr << "[!] Ошибка принятия клиентского подключения: " << e.what() << std::endl;
            }
        }
    });

    // Запускаем обработку админских подключений
    std::thread admin_thread([this]() {
        while (true) {
            try {
                auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
                admin_acceptor_.accept(*socket);
                admin_pool_.enqueue([this, socket]() {
                    handle_admin(socket);
                });
            } catch (std::exception& e) {
                std::cerr << "[!] Ошибка принятия админского подключения: " << e.what() << std::endl;
            }
        }
    });

    client_thread.join();
    admin_thread.join();
}

void CarDeliveryServer::handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    std::string client_ip = "unknown";
    
    try {
        auto remote_ep = socket->remote_endpoint();
        client_ip = remote_ep.address().to_string();
        std::cout << "[CLIENT] Новое подключение от " << client_ip << std::endl;

        boost::asio::streambuf buffer;
        boost::system::error_code read_error;
        
        // Читаем заголовки запроса
        size_t bytes_read = boost::asio::read_until(*socket, buffer, "\r\n\r\n", read_error);
        
        if (read_error) {
            std::cerr << "[!] Ошибка чтения запроса от " << client_ip << ": " << read_error.message() << std::endl;
            return;
        }
        
        if (bytes_read == 0) {
            std::cerr << "[!] Пустой запрос от " << client_ip << std::endl;
            return;
        }
        
        std::string request{
            boost::asio::buffers_begin(buffer.data()),
            boost::asio::buffers_end(buffer.data())
        };

        // Чтение тела, если Content-Length > 0
        size_t cl_pos = request.find("Content-Length: ");
        if (cl_pos != std::string::npos) {
            size_t end = request.find("\r\n", cl_pos);
            if (end != std::string::npos) {
                try {
                    int len = std::stoi(request.substr(cl_pos + 16, end - cl_pos - 16));
                    if (len > 0) {
                        // Уже прочитали часть данных в буфер, нужно дочитать остальное
                        size_t body_start = request.find("\r\n\r\n");
                        if (body_start != std::string::npos) {
                            size_t body_received = request.length() - (body_start + 4);
                            if (body_received < len) {
                                // Дополнительно читаем тело
                                std::vector<char> body(len - body_received);
                                boost::system::error_code body_error;
                                size_t body_bytes = boost::asio::read(*socket, 
                                    boost::asio::buffer(body), body_error);
                                
                                if (!body_error && body_bytes > 0) {
                                    request += std::string(body.begin(), body.end());
                                }
                            }
                        }
                    }
                } catch (const std::exception& e) {
                    std::cerr << "[!] Ошибка парсинга Content-Length от " << client_ip << ": " << e.what() << std::endl;
                }
            }
        }

        std::string response_body;
        
        // Определяем тип запроса и вызываем соответствующий обработчик
        if (request.find("GET /cars HTTP/1.1") != std::string::npos) {
            std::cout << "[CLIENT] GET /cars от " << client_ip << std::endl;
            response_body = handle_get_cars();
        }
        else if (request.find("POST /search HTTP/1.1") != std::string::npos) {
            std::cout << "[CLIENT] POST /search от " << client_ip << std::endl;
            size_t b = request.find("\r\n\r\n");
            response_body = (b != std::string::npos)
                ? handle_post_search(request.substr(b + 4))
                : R"({"error": "No body in POST /search"})";
        }
        else if (request.find("GET /search?") != std::string::npos) {
            std::cout << "[CLIENT] GET /search от " << client_ip << std::endl;
            size_t s = request.find('?'), e = request.find(' ', s);
            response_body = (s != std::string::npos && e != std::string::npos)
                ? handle_get_search(request.substr(s + 1, e - s - 1))
                : R"({"error": "Invalid query in GET /search"})";
        }
        else if (request.find("GET /cities HTTP/1.1") != std::string::npos) {
            std::cout << "[CLIENT] GET /cities от " << client_ip << std::endl;
            response_body = handle_get_cities();
        }
        else if (request.find("GET /documents HTTP/1.1") != std::string::npos) {
            std::cout << "[CLIENT] GET /documents от " << client_ip << std::endl;
            response_body = handle_get_documents();
        }
        else if (request.find("GET /delivery HTTP/1.1") != std::string::npos) {
            std::cout << "[CLIENT] GET /delivery от " << client_ip << std::endl;
            response_body = handle_get_delivery();
        }
        else if (request.find("GET /cars/specs?") != std::string::npos) {
            std::cout << "[CLIENT] GET /cars/specs от " << client_ip << std::endl;
            size_t s = request.find('?'), e = request.find(' ', s);
            response_body = (s != std::string::npos && e != std::string::npos)
                ? handle_get_cars_specs(request.substr(s + 1, e - s - 1))
                : R"({"error": "Invalid query in GET /cars/specs"})";
        }
        else if (request.find("GET /cars/brand?") != std::string::npos) {
            std::cout << "[CLIENT] GET /cars/brand от " << client_ip << std::endl;
            size_t s = request.find('?'), e = request.find(' ', s);
            response_body = (s != std::string::npos && e != std::string::npos)
                ? handle_get_cars_brand(request.substr(s + 1, e - s - 1))
                : R"({"error": "Invalid query in GET /cars/brand"})";
        }
        else if (request.find("GET /delivery/cities HTTP/1.1") != std::string::npos) {
            std::cout << "[CLIENT] GET /delivery/cities от " << client_ip << std::endl;
            response_body = handle_get_delivery_cities();
        }
        else if (request.find("GET /delivery/process HTTP/1.1") != std::string::npos) {
            std::cout << "[CLIENT] GET /delivery/process от " << client_ip << std::endl;
            response_body = handle_get_delivery_process();
        }
        else {
            std::cout << "[CLIENT] Неизвестный запрос от " << client_ip << std::endl;
            response_body = R"({"error": "Endpoint not supported"})";
        }

        // Формируем HTTP-ответ
        std::ostringstream resp;
        resp << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: application/json; charset=utf-8\r\n"
             << "Content-Length: " << response_body.size() << "\r\n"
             << "Connection: close\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "\r\n"
             << response_body;
             
        std::string response_str = resp.str();
        
        // Отправляем ответ
        boost::system::error_code write_error;
        boost::asio::write(*socket, boost::asio::buffer(response_str), write_error);
        
        if (write_error) {
            std::cerr << "[!] Ошибка отправки ответа " << client_ip << ": " << write_error.message() << std::endl;
        } else {
            std::cout << "[✓] Клиентский запрос от " << client_ip << " обработан (" << response_body.size() << " байт)\n";
        }
        
    } catch (std::exception& e) {
        std::cerr << "[!] Критическая ошибка обработки клиента " << client_ip << ": " << e.what() << std::endl;
        
        // Пытаемся отправить ошибку клиенту
        try {
            std::string error_response = 
                "HTTP/1.1 500 Internal Server Error\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: 47\r\n"
                "Connection: close\r\n"
                "\r\n"
                R"({"error": "Internal server error occurred"})";
                
            boost::asio::write(*socket, boost::asio::buffer(error_response));
        } catch (...) {
            // Игнорируем ошибки при отправке ошибки
        }
    }
}

void CarDeliveryServer::handle_admin(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    std::string client_ip = "unknown";
    
    try {
        auto remote_ep = socket->remote_endpoint();
        client_ip = remote_ep.address().to_string();
        std::cout << "[ADMIN] Новое подключение от " << client_ip << std::endl;

        boost::asio::streambuf buffer;
        boost::system::error_code read_error;
        
        // Читаем заголовки запроса
        size_t bytes_read = boost::asio::read_until(*socket, buffer, "\r\n\r\n", read_error);
        
        if (read_error) {
            std::cerr << "[!] Ошибка чтения админского запроса от " << client_ip << ": " << read_error.message() << std::endl;
            return;
        }
        
        if (bytes_read == 0) {
            std::cerr << "[!] Пустой админский запрос от " << client_ip << std::endl;
            return;
        }
        
        std::string request{
            boost::asio::buffers_begin(buffer.data()),
            boost::asio::buffers_end(buffer.data())
        };

        // Чтение тела, если Content-Length > 0
        size_t cl_pos = request.find("Content-Length: ");
        if (cl_pos != std::string::npos) {
            size_t end = request.find("\r\n", cl_pos);
            if (end != std::string::npos) {
                try {
                    int len = std::stoi(request.substr(cl_pos + 16, end - cl_pos - 16));
                    if (len > 0) {
                        // Уже прочитали часть данных в буфер, нужно дочитать остальное
                        size_t body_start = request.find("\r\n\r\n");
                        if (body_start != std::string::npos) {
                            size_t body_received = request.length() - (body_start + 4);
                            if (body_received < len) {
                                // Дополнительно читаем тело
                                std::vector<char> body(len - body_received);
                                boost::system::error_code body_error;
                                size_t body_bytes = boost::asio::read(*socket, 
                                    boost::asio::buffer(body), body_error);
                                
                                if (!body_error && body_bytes > 0) {
                                    request += std::string(body.begin(), body.end());
                                }
                            }
                        }
                    }
                } catch (const std::exception& e) {
                    std::cerr << "[!] Ошибка парсинга Content-Length в админском запросе от " << client_ip << ": " << e.what() << std::endl;
                }
            }
        }

        std::string response_body;
        
        // Обрабатываем админские запросы
        if (request.find("POST /admin/login HTTP/1.1") != std::string::npos) {
            std::cout << "[ADMIN] POST /admin/login от " << client_ip << std::endl;
            size_t b = request.find("\r\n\r\n");
            response_body = (b != std::string::npos)
                ? handle_post_admin_login(request.substr(b + 4))
                : R"({"error": "No body in POST /admin/login"})";
        }
        else {
            std::cout << "[ADMIN] Неизвестный запрос от " << client_ip << std::endl;
            response_body = R"({"error": "Admin endpoint not supported"})";
        }

        // Формируем HTTP-ответ
        std::ostringstream resp;
        resp << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: application/json; charset=utf-8\r\n"
             << "Content-Length: " << response_body.size() << "\r\n"
             << "Connection: close\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "\r\n"
             << response_body;
             
        std::string response_str = resp.str();
        
        // Отправляем ответ
        boost::system::error_code write_error;
        boost::asio::write(*socket, boost::asio::buffer(response_str), write_error);
        
        if (write_error) {
            std::cerr << "[!] Ошибка отправки админского ответа " << client_ip << ": " << write_error.message() << std::endl;
        } else {
            std::cout << "[✓] Админский запрос от " << client_ip << " обработан (" << response_body.size() << " байт)\n";
        }
        
    } catch (std::exception& e) {
        std::cerr << "[!] Критическая ошибка обработки админа " << client_ip << ": " << e.what() << std::endl;
        
        // Пытаемся отправить ошибку клиенту
        try {
            std::string error_response = 
                "HTTP/1.1 500 Internal Server Error\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: 47\r\n"
                "Connection: close\r\n"
                "\r\n"
                R"({"error": "Internal server error occurred"})";
                
            boost::asio::write(*socket, boost::asio::buffer(error_response));
        } catch (...) {
            // Игнорируем ошибки при отправке ошибки
        }
    }
}