// server.cpp (обновлённая функция handle_client)
void CarDeliveryServer::handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket, bool is_admin) {
    try {
        auto remote_ep = socket->remote_endpoint();
        std::string client_ip = remote_ep.address().to_string();

        std::cout << "[+] Новое " << (is_admin ? "АДМИН" : "КЛИЕНТ") 
                  << "-подключение от " << client_ip << std::endl;

        boost::asio::streambuf buffer;
        boost::asio::read_until(*socket, buffer, "\r\n\r\n");

        std::string request{
            std::istreambuf_iterator<char>(&buffer),
            std::istreambuf_iterator<char>()
        };

        // Парсим HTTP-запрос
        HttpRequest http_request = HttpParser::parse_request(request);
        
        // Обрабатываем запрос
        HttpResponse response;
        if (is_admin) {
            response = handle_admin_request(http_request);
        } else {
            response = handle_client_request(http_request);
        }

        // Отправляем ответ
        std::string http_response_str = response.to_string();
        boost::asio::write(*socket, boost::asio::buffer(http_response_str));
        
        std::cout << "[✓] " << http_request.method << " " << http_request.path 
                  << " от " << client_ip << " обработан (статус: " 
                  << response.status_code << ")\n";

    } catch (std::exception& e) {
        std::cerr << "[!] Ошибка при обработке " 
                  << (is_admin ? "админ" : "клиент") 
                  << "-запроса: " << e.what() << std::endl;
                  
        // Отправляем ошибку клиенту
        try {
            HttpResponse error_response = HttpResponse::error_response(500, "Internal Server Error");
            std::string error_str = error_response.to_string();
            boost::asio::write(*socket, boost::asio::buffer(error_str));
        } catch (...) {
            // Игнорируем ошибки при отправке ошибки
        }
    }
}