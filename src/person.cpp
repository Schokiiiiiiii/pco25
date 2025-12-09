#include "person.h"
#include "bike.h"
#include <random>
#include <pcosynchro/pcothread.h>

BikingInterface* Person::binkingInterface = nullptr;
std::array<BikeStation*, NB_SITES_TOTAL> Person::stations{};


Person::Person(unsigned int _id) : id(_id), homeSite(0), currentSite(0) {
    static thread_local std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, Bike::nbBikeTypes - 1);
    preferredType = dist(rng);

    if (binkingInterface) {
        log(QString("Person %1, préfère type %2")
                .arg(id).arg(preferredType));
    }
}

void Person::setStations(const std::array<BikeStation*, NB_SITES_TOTAL>& _stations){
    Person::stations = _stations;
}

void Person::setInterface(BikingInterface* _binkingInterface) {
    binkingInterface = _binkingInterface;
}

void Person::run() {

    // modified - Fabien

    // loop until we are requested to stop
    while (!PcoThread::thisThread()->stopRequested()) {

        // 1. wait for bike on site I
        Bike* currentBike = takeBikeFromSite(homeSite);

        // if a stop was requested, stop here
        if (PcoThread::thisThread()->stopRequested())
            break;

        // 2. go to site J /= I
        const unsigned int siteJ = chooseOtherSite(homeSite);
        bikeTo(siteJ, currentBike);

        // 3. wait for born on site J to free up
        depositBikeAtSite(siteJ, currentBike);
        currentSite = siteJ;

        // if a stop was requested, stop here
        if (PcoThread::thisThread()->stopRequested())
            break;

        // 4. chose a site k to walk to
        unsigned int siteK = currentSite;

        // we want K to be different from both I and J
        while (siteK == currentSite)
            siteK = chooseOtherSite(homeSite);

        // walk to site k
        walkTo(siteK);
        currentSite = siteK;

        // 5. go back to I via bike
        currentBike = takeBikeFromSite(currentSite);

        // if a stop was requested, stop here
        if (PcoThread::thisThread()->stopRequested())
            break;

        // go back to I and deposit the bike
        bikeTo(homeSite, currentBike);
        depositBikeAtSite(homeSite, currentBike);
    }
}

Bike* Person::takeBikeFromSite(const unsigned int _site) {
    Bike * bike = nullptr; // just to silence compiler warnings

    // modified - Fabien

    // take bike from station
    bike = stations[_site]->getBike(preferredType);

    // update graphical interface
    if (binkingInterface) {
        binkingInterface->setBikes(_site, stations[_site]->nbBikes());
    }

    return bike;
}

void Person::depositBikeAtSite(const unsigned int _site, Bike* _bike) {

    // modified - Fabien

    // put bike to station
    stations[_site]->putBike(_bike);

    // update graphical interface
    if (binkingInterface) {
        binkingInterface->setBikes(_site, stations[_site]->nbBikes());
    }
}

void Person::bikeTo(unsigned int _dest, Bike* _bike) {
    unsigned int t = bikeTravelTime();
    if (binkingInterface) {
        binkingInterface->travel(id, currentSite, _dest, t);
    }
    currentSite = _dest;
}

void Person::walkTo(unsigned int _dest) {
    unsigned int t = walkTravelTime();
    if (binkingInterface) {
        binkingInterface->walk(id, currentSite, _dest, t);
    }
    currentSite = _dest;
}

unsigned int Person::chooseOtherSite(unsigned int _from) const {
    return randomSiteExcept(NBSITES, _from);
}

unsigned int Person::bikeTravelTime() const {
    return randomTravelTimeMs() + 1000;
}

unsigned int Person::walkTravelTime() const {
    return randomTravelTimeMs() + 2000;
}

void Person::log(const QString& msg) const {
    if (binkingInterface) {
        binkingInterface->consoleAppendText(id, msg);
    }
}

