// http_parser.cpp
#include "http_parser.hpp"
#include <algorithm>
#include <cctype>

// Реализация HttpRequest
std::string HttpRequest::get_header(const std::string& name) const {
    auto it = headers.find(name);
    return it != headers.end() ? it->second : "";
}

bool HttpRequest::has_header(const std::string& name) const {
    return headers.find(name) != headers.end();
}

bool HttpRequest::is_json_content() const {
    std::string content_type = get_header("Content-Type");
    return content_type.find("application/json") != std::string::npos;
}

// Реализация HttpResponse
void HttpResponse::set_content_type(const std::string& type) {
    headers["Content-Type"] = type;
}

void HttpResponse::set_json_content() {
    set_content_type("application/json; charset=utf-8");
}

std::string HttpResponse::to_string() const {
    std::ostringstream response;
    
    // Статусная строка
    response << version << " " << status_code << " " << status_text << "\r\n";
    
    // Заголовки
    for (const auto& header : headers) {
        response << header.first << ": " << header.second << "\r\n";
    }
    
    // Тело
    if (!body.empty()) {
        response << "Content-Length: " << body.length() << "\r\n";
    }
    
    response << "\r\n" << body;
    
    return response.str();
}

HttpResponse HttpResponse::json_response(const std::string& json_body, unsigned int status) {
    HttpResponse response;
    response.status_code = status;
    response.status_text = (status == 200) ? "OK" : 
                          (status == 400) ? "Bad Request" :
                          (status == 404) ? "Not Found" :
                          (status == 500) ? "Internal Server Error" : "Unknown";
    response.set_json_content();
    response.body = json_body;
    return response;
}

HttpResponse HttpResponse::error_response(unsigned int status, const std::string& message) {
    std::string json_error = R"({"error": ")" + message + R"("})";
    return json_response(json_error, status);
}

// Реализация HttpParser
HttpRequest HttpParser::parse_request(const std::string& raw_request) {
    HttpRequest request;
    std::istringstream stream(raw_request);
    std::string line;
    bool is_body = false;
    
    // Первая строка - request line
    if (std::getline(stream, line) && !line.empty()) {
        parse_request_line(line, request);
    }
    
    // Заголовки
    while (std::getline(stream, line) && !line.empty()) {
        trim(line);
        if (line.empty()) {
            is_body = true;
            continue;
        }
        
        if (!is_body) {
            parse_header_line(line, request);
        } else {
            request.body += line + "\n";
        }
    }
    
    // Убираем лишний \n в конце тела
    if (!request.body.empty() && request.body.back() == '\n') {
        request.body.pop_back();
    }
    
    return request;
}

bool HttpParser::parse_request_line(const std::string& line, HttpRequest& request) {
    std::istringstream line_stream(line);
    line_stream >> request.method >> request.path >> request.version;
    
    // Убираем ? и параметры из пути
    size_t query_pos = request.path.find('?');
    if (query_pos != std::string::npos) {
        request.path = request.path.substr(0, query_pos);
    }
    
    return !request.method.empty() && !request.path.empty() && !request.version.empty();
}

bool HttpParser::parse_header_line(const std::string& line, HttpRequest& request) {
    auto header = split_header(line);
    if (!header.first.empty()) {
        request.headers[header.first] = header.second;
        return true;
    }
    return false;
}

void HttpParser::trim(std::string& str) {
    // Убираем \r из конца строки (для CRLF)
    if (!str.empty() && str.back() == '\r') {
        str.pop_back();
    }
    
    // Убираем пробелы с начала и конца
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), str.end());
}

std::pair<std::string, std::string> HttpParser::split_header(const std::string& line) {
    size_t colon_pos = line.find(':');
    if (colon_pos == std::string::npos) {
        return {"", ""};
    }
    
    std::string name = line.substr(0, colon_pos);
    std::string value = line.substr(colon_pos + 1);
    
    trim(name);
    trim(value);
    
    return {name, value};
}