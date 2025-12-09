#include "bikestation.h"

BikeStation::BikeStation(const int _capacity) : capacity(_capacity) { }

BikeStation::~BikeStation() {
    ending();
}

void BikeStation::putBike(Bike* _bike){

    // modified - Fabien

    // lock mutex
    mutex.lock();

    // if bike station full, wait in line to put bike
    while (nbBikesUnprotected() >= capacity && !stopped) {
        auto *cond = new PcoConditionVariable();
        bikePut.emplace(cond);
        cond->wait(&mutex);
        delete cond;
    }

    // if bike station is stopped, return
    if (stopped) {
        mutex.unlock();
        return;
    }

    // add bike to the correct bike type queue
    bikesPerType[_bike->bikeType].push(_bike);

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
    while (bikesPerType[_bikeType].empty() && !stopped) {
        auto *cond = new PcoConditionVariable();
        bikeGetPerType[_bikeType].emplace(cond);
        cond->wait(&mutex);
        delete cond;
    }

    // if bike station is stopped, return
    if (stopped) {
        mutex.unlock();
        return nullptr;
    }

    // take the bike type
    Bike* bike = bikesPerType[_bikeType].front();
    bikesPerType[_bikeType].pop();

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

    // modified - Aymeric

    std::vector<Bike*> result; // remaining bikes that will be returned
    mutex.lock();

    for (Bike* bike : _bikesToAdd) {
        if (nbBikesUnprotected() < capacity) { // if there's any space left
            bikesPerType[bike->bikeType].push(bike);

            // tell people who are waiting to retrieve this type of bike
            // in theory this should not happen since this function is only called from the van
            if (!bikeGetPerType[bike->bikeType].empty()) {
                bikeGetPerType[bike->bikeType].front()->notifyOne();
                bikeGetPerType[bike->bikeType].pop();
            }
        } else result.push_back(bike);
    }

    mutex.unlock();
    return result;
}

std::vector<Bike*> BikeStation::getBikes(size_t _nbBikes) {

    // modified - Aymeric

    std::vector<Bike*> result; // bikes retrieved from the station
    mutex.lock();

    // retrieve bikes until there is no more to retrieve, or we have retrieved enough
    for (size_t type = 0; type < Bike::nbBikeTypes; ++type) {
        while (!bikesPerType[type].empty() && _nbBikes) {
            result.push_back(bikesPerType[type].front());
            bikesPerType[type].pop();
            --_nbBikes;

            // tell people who are waiting to deposit this type of bike
            // in theory this should not happen since this function is only called from the van
            if (!bikePut.empty()) {
                bikePut.front()->notifyOne();
                bikePut.pop();
            }
        }
    }

    mutex.unlock();
    return result;
}

size_t BikeStation::countBikesOfType(size_t type) {

    // modified - Fabien

    mutex.lock();

    // we have a data race on the .size()
    const size_t nb = bikesPerType[type].size();

    mutex.unlock();

    // return the type size
    return nb;
}

size_t BikeStation::nbBikesUnprotected() {

    // modified - Fabien

    // get the number of each type of bike and add up
    size_t nb = 0;
    for (const auto& types : bikesPerType) {
        nb += types.size();
    }

    return nb;
}

size_t BikeStation::nbBikes() {

    // modified - Fabien

    mutex.lock();

    // get the number of each type of bike and add up
    size_t nb = nbBikesUnprotected();

    mutex.unlock();

    return nb;
}

size_t BikeStation::nbSlots() {
    return capacity;
}

void BikeStation::ending() {

    // modified - Fabien

    // lock mutex since we don't want sizes to change when we free all
    mutex.lock();

    // turn stopped to true so threads can leave
    stopped = true;

    // wake up all getters
    for (auto& type : bikeGetPerType) {
        while (!type.empty()) {
            type.front()->notifyOne();
            type.pop();
        }
    }

    // wake up all putters
    while (!bikePut.empty()) {
        bikePut.front()->notifyOne();
        bikePut.pop();
    }

    // unlock mutex after all operations are done
    mutex.unlock();
}
