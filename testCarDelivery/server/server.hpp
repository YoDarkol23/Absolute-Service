/**
 * @file server/server.hpp
 * @brief Объявления классов сервера.
 * 
 * Использует два порта:
 * - 8080 — для обычных клиентов
 * - 8081 — для администраторов
 * Аутентификация админа происходит на уровне приложения (/admin/login).
 */

#pragma once
#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>

class ThreadPool {
public:
    explicit ThreadPool(size_t threads);
    template<class F> void enqueue(F&& f);
    ~ThreadPool();

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop = false;
};

class CarDeliveryServer {
public:
    explicit CarDeliveryServer(unsigned short client_port = 8080, unsigned short admin_port = 8081);
    void run();

private:
    void handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void handle_admin(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor client_acceptor_;
    boost::asio::ip::tcp::acceptor admin_acceptor_;
    ThreadPool client_pool_{6}; // 6 потоков для всех запросов
    ThreadPool admin_pool_{2};  // 2 потока для админ-запросов
};