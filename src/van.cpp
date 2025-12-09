#include "van.h"

BikingInterface* Van::binkingInterface = nullptr;
std::array<BikeStation*, NB_SITES_TOTAL> Van::stations{};

// Constructeur
Van::Van(unsigned int _id) : id(_id), currentSite(DEPOT_ID) {}

void Van::run() {
    while (true /*TODO: clean stop*/) {
        size_t a = std::min(2, stations[DEPOT_ID]->nbBikes());
        loadAtDepot(a);
        for (unsigned int s = 0; s < NBSITES; ++s) {
            driveTo(s);
            balanceSite(s, a); // je lui donne a aussi
        }
        returnToDepot();
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

void Van::loadAtDepot(size_t a) {
    driveTo(DEPOT_ID);

    // If possible, at least 2 bikes are loaded

    // is there  space left in the van? y -> are we able to fetch any bike from the depot? y -> bikes is pasted at the end of cargo

    if (cargo.size() < VAN_CAPACITY) // python style baby
        if (std::vector<Bike *> bikes = stations[DEPOT_ID]->getBikes(a); bikes.size())
            cargo.insert(cargo.end(), bikes.begin(), bikes.end());


    if (binkingInterface) binkingInterface->setBikes(DEPOT_ID, stations[DEPOT_ID]->nbBikes());
}


void Van::balanceSite(unsigned int _site, size_t& a)
{
    // TODO: implement this method

    size_t nbBikes = stations[_site]->nbBikes();,

    if (nbBikes > BORNES - 2) {
        size_t c = std::min(nbBikes - BORNES + 2, VAN_CAPACITY - a); // nombre de vélos à retirer de _site
        std::vector<Bike *> bikes = stations[_site]->getBikes(c);
        cargo.insert(cargo.end(), bikes.begin(), bikes.end());
        a += c;
    }
    else if (nbBikes < BORNES - 2) {
        size_t c = std::min(BORNES - nbBikes - 2, a); // nombre de vélos à ajouter à _site
        uint bikesDropped = 0;
        for (size_t type = 0; type < Bike::nbBikeTypes; ++type) {
            if (!stations[_site]->countBikesOfType(type)) {
                for (size_t i = 0; i < cargo.size(); ++i) {

                }
            }
        }
    }

    if (binkingInterface) binkingInterface->setBikes(DEPOT_ID, stations[DEPOT_ID]->nbBikes()); // Keep somewhere for GUI
}

void Van::returnToDepot() {
    driveTo(DEPOT_ID);

    // car no go space, car go road.
    size_t cargoCount = cargo.size();

    // TODO: implement this method. If the van carries bikes, then leave them

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

