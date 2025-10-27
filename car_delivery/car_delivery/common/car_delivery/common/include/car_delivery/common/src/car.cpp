#include "car.h"

Car::Car(const std::string& model, int year)
    : model_(model), year_(year) {}

std::string Car::getInfo() const {
    return model_ + " (" + std::to_string(year_) + ")";
}
