#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include "../common/utils.hpp"

// Вспомогательная функция для создания тестового файла
void createTestFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    file << content;
    file.close();
}

// Тест для read_file
TEST(UtilsTest, ReadFileExistingFile) {
    const std::string test_filename = "test_read.txt";
    const std::string test_content = "Hello, World!\nTest content.";
    
    // Создаем тестовый файл
    createTestFile(test_filename, test_content);
    
    // Читаем файл
    std::string result = read_file(test_filename);
    
    // Проверяем результат
    EXPECT_EQ(result, test_content);
    
    // Удаляем тестовый файл
    remove(test_filename.c_str());
}

TEST(UtilsTest, ReadFileNonExistingFile) {
    std::string result = read_file("non_existing_file.txt");
    
    // Проверяем, что возвращается JSON с ошибкой
    EXPECT_TRUE(result.find("error") != std::string::npos);
    EXPECT_TRUE(result.find("File not found") != std::string::npos);
}

TEST(UtilsTest, ReadFileEmptyFile) {
    const std::string test_filename = "test_empty.txt";
    
    // Создаем пустой файл
    std::ofstream file(test_filename);
    file.close();
    
    // Читаем файл
    std::string result = read_file(test_filename);
    
    // Проверяем результат
    EXPECT_TRUE(result.empty());
    
    // Удаляем тестовый файл
    remove(test_filename.c_str());
}

// Тест для extract_json_from_response
TEST(UtilsTest, ExtractJsonFromResponseWithDoubleNewline) {
    std::string http_response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 20\r\n"
        "\r\n"
        "{\"key\": \"value\"}";
    
    std::string result = extract_json_from_response(http_response);
    EXPECT_EQ(result, "{\"key\": \"value\"}");
}

TEST(UtilsTest, ExtractJsonFromResponseWithSingleNewline) {
    std::string http_response = 
        "HTTP/1.1 200 OK\n"
        "Content-Type: application/json\n"
        "Content-Length: 20\n"
        "\n"
        "{\"key\": \"value\"}";
    
    std::string result = extract_json_from_response(http_response);
    EXPECT_EQ(result, "{\"key\": \"value\"}");
}

TEST(UtilsTest, ExtractJsonFromResponseNoHeaders) {
    std::string http_response = "{\"key\": \"value\"}";
    
    std::string result = extract_json_from_response(http_response);
    EXPECT_EQ(result, "{\"key\": \"value\"}");
}

TEST(UtilsTest, ExtractJsonFromResponseEmpty) {
    std::string http_response = "";
    
    std::string result = extract_json_from_response(http_response);
    EXPECT_TRUE(result.empty());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}