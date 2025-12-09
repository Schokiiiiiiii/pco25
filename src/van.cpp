#include "van.h"

BikingInterface* Van::binkingInterface = nullptr;
std::array<BikeStation*, NB_SITES_TOTAL> Van::stations{};

// Constructeur
Van::Van(unsigned int _id) : id(_id), currentSite(DEPOT_ID) {}

void Van::run() {
    // TODO fix this, as ending() returns void and not bool

    while (!stations[currentSite]->ending()) {
        loadAtDepot();
        for (unsigned int s = 0; s < NBSITES; ++s) {
            driveTo(s);
            balanceSite(s);
        }
        returnToDepot();
        usleep(300000); // 300000 micro-secondes est égal à 0.3 secondes, ce qui est bien assez pour le repos des employés
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

    unsigned int travelTime = randomTravelTimeMs();
    if (binkingInterface) binkingInterface->vanTravel(currentSite, _dest, travelTime);

    currentSite = _dest;
}

void Van::loadAtDepot() {
    driveTo(DEPOT_ID);

    // is there space left in the van? y -> are we able to fetch any bike from the depot? y -> bikes is pasted at the end of cargo
    if (cargo.size() < VAN_CAPACITY) // in theory the cargo is empty at the beginning of the day
        if (std::vector<Bike *> bikes = stations[DEPOT_ID]->getBikes(std::min((size_t)2, stations[DEPOT_ID]->nbBikes())); bikes.size())
            // If possible, at least 2 bikes are loaded
            cargo.insert(cargo.end(), bikes.begin(), bikes.end());

    if (binkingInterface) binkingInterface->setBikes(DEPOT_ID, stations[DEPOT_ID]->nbBikes());
}


void Van::balanceSite(unsigned int _site)
{
    size_t nbBikes = stations[_site]->nbBikes();

    if (nbBikes > BORNES - 2) {
        // nombre de vélos à retirer de _site
        std::vector<Bike *> bikes = stations[_site]->getBikes(std::min(nbBikes - BORNES + 2, VAN_CAPACITY - cargo.size()));
        // il est possible qu'on ne puisse pas en ajouter au van
        cargo.insert(cargo.end(), bikes.begin(), bikes.end());
    }
    else if (nbBikes < BORNES - 2) {
        size_t c = std::min(BORNES - nbBikes - 2, cargo.size()); // nombre de vélos à ajouter à _site
        uint bikesDropped = 0;
        for (size_t type = 0; type < Bike::nbBikeTypes; ++type) { // pour chaque type de vélo
            if (!stations[_site]->countBikesOfType(type)) { // si y en a pas dans le site
                if (Bike* bike = takeBikeFromCargo(type); bike != nullptr) { // et qu'on a pu en prendre un dans le van
                    stations[_site]->putBike(bike); // alors on le mets dans le site
                    ++bikesDropped;
                }
                if (bikesDropped == c) break;
            }
        }
        while (bikesDropped < c && cargo.size()) { // si on en a pas mis assez et qu'il nous en reste
            stations[_site]->putBike(cargo.back()); // on en met sans les choisir
            cargo.pop_back();
            ++bikesDropped;
        }
    }
    // si nbBikes == BORNES - 2 on a rien besoin de faire

    if (binkingInterface) binkingInterface->setBikes(DEPOT_ID, stations[DEPOT_ID]->nbBikes()); // Keep somewhere for GUI
}

void Van::returnToDepot() {
    driveTo(DEPOT_ID);

    // If the van carries bikes, then leave them
    if (cargo = stations[DEPOT_ID]->addBikes(cargo); cargo.size())
        log("Couldn't return all bikes to the depot");

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

