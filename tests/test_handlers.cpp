#define BOOST_TEST_MODULE HandlersTest
#include <boost/test/included/unit_test.hpp>

// Подключаем заголовочные файлы, содержащие тестируемую логику
#include "../server/handlers.hpp"
#include "../common/utils.hpp" // Может понадобиться для JSON операций

// --- Вспомогательные функции для тестов ---
nlohmann::json create_sample_cars_json() {
    return nlohmann::json::array({
        {
            {"id", 1},
            {"brand", "Toyota"},
            {"model", "Camry"},
            {"year", 2020},
            {"engine_volume", 2.5},
            {"fuel_type", "Gasoline"},
            {"transmission", "Automatic"},
            {"price", 25000}
        },
        {
            {"id", 2},
            {"brand", "Honda"},
            {"model", "Civic"},
            {"year", 2018},
            {"engine_volume", 1.5},
            {"fuel_type", "Gasoline"},
            {"transmission", "Manual"},
            {"price", 18000}
        },
        {
            {"id", 3},
            {"brand", "Ford"},
            {"model", "F-150"},
            {"year", 2022},
            {"engine_volume", 3.5},
            {"fuel_type", "Gasoline"},
            {"transmission", "Automatic"},
            {"price", 35000}
        }
    });
}

// Функция для проверки равенства двух JSON массивов/объектов
bool json_arrays_equal(const nlohmann::json& arr1, const nlohmann::json& arr2) {
    if (arr1.size() != arr2.size()) {
        return false;
    }
    for (size_t i = 0; i < arr1.size(); ++i) {
        if (arr1[i] != arr2[i]) {
            return false;
        }
    }
    return true;
}

// --- Тесты ---

BOOST_AUTO_TEST_SUITE(HandlersSuite)

BOOST_AUTO_TEST_CASE(fetch_cars_by_specs_EmptyList_ReturnsEmpty) {
    nlohmann::json empty_list = nlohmann::json::array();
    nlohmann::json specs = {{"brand", "Toyota"}};

    auto result = fetch_cars_by_specs(empty_list, specs);

    BOOST_REQUIRE(result.is_array());
    BOOST_CHECK_EQUAL(result.size(), 0U);
}

BOOST_AUTO_TEST_CASE(fetch_cars_by_specs_MatchFound_ReturnsMatched) {
    auto cars = create_sample_cars_json();
    nlohmann::json expected_result = nlohmann::json::array({
        cars[0] // Toyota Camry 2020
    });
    nlohmann::json specs = {{"brand", "Toyota"}, {"year", 2020}};

    auto result = fetch_cars_by_specs(cars, specs);

    BOOST_REQUIRE(result.is_array());
    BOOST_CHECK(json_arrays_equal(result, expected_result));
}

BOOST_AUTO_TEST_CASE(fetch_cars_by_specs_NoMatch_ReturnsEmpty) {
    auto cars = create_sample_cars_json();
    nlohmann::json specs = {{"brand", "BMW"}, {"year", 2025}};

    auto result = fetch_cars_by_specs(cars, specs);

    BOOST_REQUIRE(result.is_array());
    BOOST_CHECK_EQUAL(result.size(), 0U);
}

BOOST_AUTO_TEST_CASE(fetch_cars_by_specs_PartialMatch_OnlyExactMatched) {
    auto cars = create_sample_cars_json();
    // Ищем только по brand, должен вернуть все Toyota
    nlohmann::json specs = {{"brand", "Toyota"}};
    nlohmann::json expected_result = nlohmann::json::array({
        cars[0] // Toyota Camry 2020
    });

    auto result = fetch_cars_by_specs(cars, specs);

    BOOST_REQUIRE(result.is_array());
    // В данном случае ожидаем одно совпадение по бренду, но specs не включает год
    // fetch_cars_by_specs ищет точное совпадение для ВСЕХ указанных полей specs
    // Поэтому результат должен быть пустым, если specs {"brand": "Toyota"} не является полным объектом машины
    // Нужно уточнить поведение specs. Specs - это шаблон, по которому ищутся поля.
    // fetch_cars_by_specs ищет машины, у которых все поля из specs совпадают со своими значениями.
    // Поэтому {"brand": "Toyota"} означает найти машины, у которых brand="Toyota".
    // Он не требует, чтобы у машины были все остальные поля из specs, только чтобы совпадали указанные.
    // Исправленный ожидаемый результат:
    nlohmann::json expected_result_partial = nlohmann::json::array();
    for (const auto& car : cars) {
         if (car.contains("brand") && car["brand"] == "Toyota") {
             expected_result_partial.push_back(car);
         }
    }

    BOOST_REQUIRE(result.is_array());
    BOOST_CHECK(json_arrays_equal(result, expected_result_partial)); // Теперь проверяем все Toyota
}


BOOST_AUTO_TEST_CASE(fetch_cars_by_specs_MultipleMatches_ReturnsAll) {
     auto cars = create_sample_cars_json();
     // Поиск по типу топлива - должны вернуться Civic и F-150
     nlohmann::json specs = {{"fuel_type", "Gasoline"}}; // Все в примере используют Gasoline
     nlohmann::json expected_result = cars; // Все должны совпадать по specs

     auto result = fetch_cars_by_specs(cars, specs);

     BOOST_REQUIRE(result.is_array());
     BOOST_CHECK(json_arrays_equal(result, expected_result));
}

BOOST_AUTO_TEST_CASE(fetch_cars_by_specs_NumericComparison_YearCheck) {
     auto cars = create_sample_cars_json();
     // Поиск машин старше 2020 года (не включая 2020)
     // fetch_cars_by_specs делает точное сравнение. Для диапазонов нужна другая логика.
     // Тестируем точное числовое совпадение.
     nlohmann::json specs = {{"year", 2018}};
     nlohmann::json expected_result = nlohmann::json::array({cars[1]}); // Honda Civic 2018

     auto result = fetch_cars_by_specs(cars, specs);

     BOOST_REQUIRE(result.is_array());
     BOOST_CHECK(json_arrays_equal(result, expected_result));
}

// Тест для случая, когда в specs есть ключ, которого нет ни в одной машине
BOOST_AUTO_TEST_CASE(fetch_cars_by_specs_KeyNotFound_ReturnsEmpty) {
     auto cars = create_sample_cars_json();
     nlohmann::json specs = {{"color", "Red"}}; // Поле color отсутствует в cars

     auto result = fetch_cars_by_specs(cars, specs);

     BOOST_REQUIRE(result.is_array());
     BOOST_CHECK_EQUAL(result.size(), 0U);
}


BOOST_AUTO_TEST_SUITE_END()
