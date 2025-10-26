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

    client_thread.join();
    admin_thread.join();
}

void CarDeliveryServer::handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket, bool is_admin) {
    try {
        auto remote_ep = socket->remote_endpoint();
        std::string client_ip = remote_ep.address().to_string();

        std::cout << "[+] Новое " << (is_admin ? "АДМИН" : "КЛИЕНТ") 
                  << "-подключение от " << client_ip << std::endl;

        boost::asio::streambuf buffer;
        boost::asio::read_until(*socket, buffer, "\r\n\r\n");

        // ИСПРАВЛЕНО: uniform initialization {}
        std::string request{
            std::istreambuf_iterator<char>(&buffer),
            std::istreambuf_iterator<char>()
        };

        std::string response_body;
        if (is_admin) {
            response_body = handle_admin_request(request);
        } else {
            if (request.find("GET /cars") != std::string::npos) {
                response_body = handle_get_cars();
            } else {
                response_body = R"({"error": "Endpoint not supported. Try GET /cars"})";
            }
        }

        std::ostringstream http_response;
        http_response << "HTTP/1.1 200 OK\r\n"
                      << "Content-Type: application/json\r\n"
                      << "Content-Length: " << response_body.size() << "\r\n"
                      << "Connection: close\r\n\r\n"
                      << response_body;

        boost::asio::write(*socket, boost::asio::buffer(http_response.str()));
        std::cout << "[✓] Запрос от " << client_ip << " обработан успешно\n";

    } catch (std::exception& e) {
        std::cerr << "[!] Ошибка при обработке " 
                  << (is_admin ? "админ" : "клиент") 
                  << "-запроса: " << e.what() << std::endl;
    }
}