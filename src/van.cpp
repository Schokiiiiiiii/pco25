#include "van.h"
#include <pcosynchro/pcothread.h>
#include <thread>
#include <chrono>

BikingInterface* Van::binkingInterface = nullptr;
std::array<BikeStation*, NB_SITES_TOTAL> Van::stations{};

// Constructor
Van::Van(unsigned int _id) : id(_id), currentSite(DEPOT_ID) {}

void Van::run() {

    // modified - Aymeric

    // We test at each site to see if the simulation is about to stop
    while (!PcoThread::thisThread()->stopRequested()) {
        loadAtDepot();
        for (unsigned int s = 0; s < NBSITES; ++s) {
            driveTo(s);
            balanceSite(s);
        }
        returnToDepot();
        // 0.3 seconds of sleep a day keeps the doctor away
        PcoThread::thisThread()->usleep(300000);
    }
    log("Van s'arrête proprement");
}

void Van::setInterface(BikingInterface* _binkingInterface){
    binkingInterface = _binkingInterface;
}

void Van::setStations(const std::array<BikeStation*, NB_SITES_TOTAL>& _stations) {
    stations = _stations;
}

void Van::log(const QString& msg) const {
    if (binkingInterface) binkingInterface->consoleAppendText(0, msg);
}

void Van::driveTo(unsigned int _dest) {
    if (currentSite == _dest) return;

    // random transport waiting time
    unsigned int travelTime = randomTravelTimeMs();
    if (binkingInterface) binkingInterface->vanTravel(currentSite, _dest, travelTime);

    currentSite = _dest;
}

void Van::loadAtDepot() {

    // modified - Aymeric

    driveTo(DEPOT_ID);

    // is there any space left in the van? y -> did we manage to fetch any bike from the storage? y -> we add bikes to cargo
    if (cargo.size() < VAN_CAPACITY) // en théorie le van est vide à la fin de la journée
        if (std::vector<Bike *> bikes = stations[DEPOT_ID]->getBikes(std::min((size_t)2, stations[DEPOT_ID]->nbBikes())); bikes.size())
            // if possible, at least 2 bikes are loaded in the van
            cargo.insert(cargo.end(), bikes.begin(), bikes.end());

    if (binkingInterface) binkingInterface->setBikes(DEPOT_ID, stations[DEPOT_ID]->nbBikes());
}


void Van::balanceSite(unsigned int _site) {

    // modified - Aymeric

    size_t nbBikes = stations[_site]->nbBikes();

    if (nbBikes > BORNES - 2) {
        // number of bikes to be removed from _site
        std::vector<Bike *> bikes = stations[_site]->getBikes(std::min(nbBikes - BORNES + 2, VAN_CAPACITY - cargo.size()));
        // It's possible that we won't be able to add any more to the van
        cargo.insert(cargo.end(), bikes.begin(), bikes.end());
    }
    else if (nbBikes < BORNES - 2) {
        size_t c = std::min(BORNES - nbBikes - 2, cargo.size()); // number of bikes to add to _site
        uint bikesDropped = 0;
        for (size_t type = 0; type < Bike::nbBikeTypes; ++type) { // for each type of bike
            if (!stations[_site]->countBikesOfType(type)) { // if there is none at the station
                if (Bike* bike = takeBikeFromCargo(type); bike != nullptr) { // and if we could load one in the van
                    stations[_site]->putBike(bike); // then we add it to the station
                    ++bikesDropped;
                }
                if (bikesDropped == c) break;
            }
        }
        while (bikesDropped < c && cargo.size()) { // if we haven't put enough and we still have some left
            stations[_site]->putBike(cargo.back()); // we put them without choosing them
            cargo.pop_back();
            ++bikesDropped;
        }
    }
    // if nbBikes == BORNES - 2 we don't need to do anything

    if (binkingInterface) binkingInterface->setBikes(DEPOT_ID, stations[DEPOT_ID]->nbBikes()); // Keep somewhere for GUI
}

void Van::returnToDepot() {

    // modified - Aymeric

    driveTo(DEPOT_ID);

    // If the van is transporting bicycles, we try to leave them at the depot
    if (cargo = stations[DEPOT_ID]->addBikes(cargo); cargo.size())
        log("Van n'a pas pu laisser tous les vélos au dépôt");

    if (binkingInterface) binkingInterface->setBikes(DEPOT_ID, stations[DEPOT_ID]->nbBikes());
}

Bike* Van::takeBikeFromCargo(size_t type) {
    for (size_t i = 0; i < cargo.size(); ++i) {
        if (cargo[i]->bikeType == type) {
            Bike* bike = cargo[i];
            cargo[i] = cargo.back();
            cargo.pop_back();
            return bike;
        }
    }
    return nullptr;
}

