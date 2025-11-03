// ambulance.cpp
#include "ambulance.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>

// le nombre d'ambulances par défaut est 2, comme défini dans le main

static PcoMutex sickMutex; // ce mutex régit le nombre de patients malades, qui sont partagés entre les ambulances

Ambulance::Ambulance(int id, int fund, std::vector<ItemType> resourcesSupplied, std::map<ItemType, int> initialStocks)
    : Seller(fund, id), resourcesSupplied(resourcesSupplied) {
    for (auto it : resourcesSupplied)
        stocks[it] = initialStocks.count(it) ? initialStocks[it] : 0;
}

void Ambulance::run() {
    logger() << "Ambulance " << uniqueId << " starting with fund " << money << std::endl;

    while (true) {
        clock->worker_wait_day_start();
        if (PcoThread::thisThread()->stopRequested()) break;

        sendPatients();

        clock->worker_end_day();
    }

    logger() << "Ambulance " << uniqueId << " stopping with fund " << money << std::endl;
}

void Ambulance::sendPatients() {
    // Choisir un hôpital au hasard
    auto *hospital = chooseRandomSeller(hospitals);

    // Déterminer le nombre de patients à envoyer
    int nbPatientsToTransfer = 1 + rand() % 5;

    if (int salary = getEmployeeSalary(EmployeeType::EmergencyStaff); money >= salary) {

        int nbPatientsTransferred = hospital->transfer(ItemType::SickPatient, nbPatientsToTransfer);

        // section critique
        moneyMutex.lock();
        money -= salary; // attention, money peut augmenter et baisser en même temps donc il faut un mutex par instance
        moneyMutex.unlock();
        // fin section critique

        // section critique
        sickMutex.lock();
        stocks.at(ItemType::SickPatient) -= nbPatientsTransferred;
        sickMutex.unlock();
        // fin section critique

        insurance->invoice(getCostPerService(ServiceType::Transport), this);
        nbEmployeesPaid++;
    }

}

void Ambulance::pay(int bill) {

    // section critique
    moneyMutex.lock();
    money += bill;
    moneyMutex.unlock();
    // fin section critique
}

void Ambulance::setHospitals(std::vector<Seller *> h) { hospitals = std::move(h); } // à appeller dans hospital.cpp?

void Ambulance::setInsurance(Seller *ins) { insurance = ins; } // à appeler dans insurance.cpp?

int Ambulance::getNumberPatients() { return stocks[ItemType::SickPatient]; }
