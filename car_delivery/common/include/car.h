#pragma once
#include <string>

class Car {
public:
    Car(const std::string& model, int year);
    std::string getInfo() const;

private:
    std::string model_;
    int year_;
};
