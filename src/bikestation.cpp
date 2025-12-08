#include "bikestation.h"

BikeStation::BikeStation(const int _capacity) : capacity(_capacity) {
    for (auto &type : bikesPerType) {
        type.reserve(capacity);
    }
}

BikeStation::~BikeStation() {
    ending();
}

void BikeStation::putBike(Bike* _bike){

    // modified - Fabien

    // lock mutex
    mutex.lock();

    // if bike station full, wait in line to put bike
    while (nbBikes() >= capacity) {
        auto *cond = new PcoConditionVariable();
        bikePut.emplace(cond);
        cond->wait(&mutex);
        delete cond;
    }

    // add bike to the correct bike type queue
    bikesPerType[_bike->bikeType].push_back(_bike);

    // if getter waiting, free him depending on bike type
    if (!bikeGetPerType[_bike->bikeType].empty()) {
        bikeGetPerType[_bike->bikeType].front()->notifyOne();
        bikeGetPerType[_bike->bikeType].pop();
    }

    // unlock mutex
    mutex.unlock();
}

Bike* BikeStation::getBike(const size_t _bikeType) {

    // modified - Fabien

    // lock mutex
    mutex.lock();

    // if bike station bike type is empty, wait
    while (bikesPerType[_bikeType].empty()) {
        auto *cond = new PcoConditionVariable();
        bikeGetPerType[_bikeType].emplace(cond);
        cond->wait(&mutex);
        delete cond;
    }

    // take the bike type
    Bike* bike = bikesPerType[_bikeType].back();
    bikesPerType[_bikeType].pop_back();

    // if there are putters waiting, free the first one
    if (!bikePut.empty()) {
        bikePut.front()->notifyOne();
        bikePut.pop();
    }

    // unlock mutex
    mutex.unlock();

    // give back the bike to the citizen
    return bike;
}

std::vector<Bike*> BikeStation::addBikes(std::vector<Bike*> _bikesToAdd) {
    std::vector<Bike*> result; // Can be removed, it's just to avoid a compiler warning
    // TODO: implement this method
    return result;
}

std::vector<Bike*> BikeStation::getBikes(size_t _nbBikes) {
    std::vector<Bike*> result; // Can be removed, it's just to avoid a compiler warning
    // TODO: implement this method
    return result;
}

size_t BikeStation::countBikesOfType(size_t type) const {

    // modified - Fabien

    // return the type size
    return bikesPerType[type].size();
}

size_t BikeStation::nbBikes() {

    // modified - Fabien

    // get the number of each type of bike and add up
    size_t nb = 0;
    for (const auto& types : bikesPerType) {
        nb += types.size();
    }

    return nb;
}

size_t BikeStation::nbSlots() {
    return capacity;
}

void BikeStation::ending() {
   // TODO: implement this method
}
