#include "van.h"
#include <pcosynchro/pcothread.h>
#include <thread>
#include <chrono>

BikingInterface* Van::binkingInterface = nullptr;
std::array<BikeStation*, NB_SITES_TOTAL> Van::stations{};

// Constructeur
Van::Van(unsigned int _id) : id(_id), currentSite(DEPOT_ID) {}

void Van::run() {

    // modified - Aymeric

    // on teste à chaque site si la simulation n'est pas en train de s'arrêter
    while (!PcoThread::thisThread()->stopRequested()) {
        loadAtDepot();
        for (unsigned int s = 0; s < NBSITES; ++s) {
            driveTo(s);
            balanceSite(s);
        }
        returnToDepot();
        // c'est important d'être bien reposé pour repartir du bon pied
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

    //  temps d'attente de transport random
    unsigned int travelTime = randomTravelTimeMs();
    if (binkingInterface) binkingInterface->vanTravel(currentSite, _dest, travelTime);

    currentSite = _dest;
}

void Van::loadAtDepot() {

    // modified - Aymeric

    driveTo(DEPOT_ID);

    // a-t-on de la place dans le van? y -> avons-nous réussi à prendre des vélos du dépot? y -> on colle bikes à la fin de cargo
    if (cargo.size() < VAN_CAPACITY) // en théorie le van est vide à la fin de la journée
        if (std::vector<Bike *> bikes = stations[DEPOT_ID]->getBikes(std::min((size_t)2, stations[DEPOT_ID]->nbBikes())); bikes.size())
            // si possible, au moins 2 vélos sont chargés dans le van
            cargo.insert(cargo.end(), bikes.begin(), bikes.end());

    if (binkingInterface) binkingInterface->setBikes(DEPOT_ID, stations[DEPOT_ID]->nbBikes());
}


void Van::balanceSite(unsigned int _site) {

    // modified - Aymeric

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

    // modified - Aymeric

    driveTo(DEPOT_ID);

    // si le van transporte des vélos, on tente de les laisser au dépot
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

