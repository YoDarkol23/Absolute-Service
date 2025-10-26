/**
 * @file server/server.hpp
 * @brief Объявления классов сервера и пула потоков.
 * 
 * Содержит:
 * - ThreadPool — пул рабочих потоков для обработки запросов
 * - CarDeliveryServer — основной класс сервера
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

/**
 * Простой пул потоков.
 * Позволяет выполнять задачи в фиксированном числе потоков.
 * Используется для обработки клиентов без создания нового потока на каждый запрос.
 */
class ThreadPool {
public:
    explicit ThreadPool(size_t threads);
    template<class F> void enqueue(F&& f);
    ~ThreadPool();

private:
    std::vector<std::thread> workers;          // Рабочие потоки
    std::queue<std::function<void()>> tasks;   // Очередь задач
    std::mutex queue_mutex;                    // Защита очереди
    std::condition_variable condition;         // Уведомление потоков
    bool stop = false;                         // Флаг остановки
};

/**
 * Основной класс сервера.
 * Слушает два порта и распределяет запросы по пулам потоков.
 */
class CarDeliveryServer {
public:
    CarDeliveryServer(unsigned short client_port, unsigned short admin_port);
    void run();  // Запуск сервера

private:
    void start_acceptors();  // Запуск двух acceptor'ов
    void handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket, bool is_admin);

    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor client_acceptor_;  // Порт для клиентов (8080)
    boost::asio::ip::tcp::acceptor admin_acceptor_;   // Порт для админа (8081)
    ThreadPool client_pool_{6};  // 6 потоков для обычных клиентов
    ThreadPool admin_pool_{2};   // 2 потока для админа (приоритет!)
};