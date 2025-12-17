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
    explicit CarDeliveryServer(unsigned short port = 8080);
    void run();

private:
    void handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    ThreadPool client_pool_{6}; // 6 потоков для всех запросов
};