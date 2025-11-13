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

        boost::asio::streambuf buffer;
        boost::asio::read_until(*socket, buffer, "\r\n\r\n");
        std::string request{
            std::istreambuf_iterator<char>(&buffer),
            std::istreambuf_iterator<char>()
        };

        // Чтение тела, если Content-Length > 0
        size_t cl_pos = request.find("Content-Length: ");
        if (cl_pos != std::string::npos) {
            size_t end = request.find("\r\n", cl_pos);
            int len = std::stoi(request.substr(cl_pos + 16, end - cl_pos - 16));
            if (len > 0) {
                std::vector<char> body(len);
                boost::asio::read(*socket, boost::asio::buffer(body));
                request += std::string(body.begin(), body.end());
            }
        }

        std::string response_body;
        if (request.find("GET /cars") != std::string::npos) {
            response_body = handle_get_cars();
        }
        else if (request.find("POST /search") != std::string::npos) {
            size_t b = request.find("\r\n\r\n");
            response_body = (b != std::string::npos)
                ? handle_post_search(request.substr(b + 4))
                : R"({"error": "No body in POST /search"})";
        }
        else if (request.find("GET /search?") != std::string::npos) {
            size_t s = request.find('?'), e = request.find(' ', s);
            response_body = (s != std::string::npos && e != std::string::npos)
                ? handle_get_search(request.substr(s + 1, e - s - 1))
                : R"({"error": "Invalid query in GET /search"})";
        }
        else if (request.find("GET /cities") != std::string::npos) {
            response_body = handle_get_cities();
        }
        else if (request.find("GET /documents") != std::string::npos) {
            response_body = handle_get_documents();
        }
        else if (request.find("GET /delivery") != std::string::npos) {
            response_body = handle_get_delivery();
        }
        else if (request.find("POST /admin/login") != std::string::npos) {
            size_t b = request.find("\r\n\r\n");
            response_body = (b != std::string::npos)
                ? handle_post_admin_login(request.substr(b + 4))
                : R"({"error": "No body in POST /admin/login"})";
        }
        else {
            response_body = R"({"error": "Endpoint not supported"})";
        }

        std::ostringstream resp;
        resp << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: application/json\r\n"
             << "Content-Length: " << response_body.size() << "\r\n"
             << "Connection: close\r\n\r\n"
             << response_body;
        boost::asio::write(*socket, boost::asio::buffer(resp.str()));
        std::cout << "[✓] Запрос от " << client_ip << " обработан\n";
    } catch (std::exception& e) {
        std::cerr << "[!] Ошибка: " << e.what() << std::endl;
    }
}