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
    if (stocks[ItemType::SickPatient]) {

        // Choisir un hôpital au hasard
        auto *hospital = chooseRandomSeller(hospitals);

        // Déterminer le nombre de patients à envoyer
        int nbPatientsToTransfer = 1 + rand() % 5;
        while (nbPatientsToTransfer > stocks[ItemType::SickPatient]) nbPatientsToTransfer = 1 + rand() % 5;


        // attention, money peut augmenter et baisser en même temps donc il faut un mutex par instance
        moneyMutex.lock();
        if (int salary = getEmployeeSalary(EmployeeType::EmergencyStaff); money >= salary) {
            int nbPatientsTransferred = hospital->transfer(ItemType::SickPatient, nbPatientsToTransfer);

            money -= salary;
            moneyMutex.unlock();

            sickMutex.lock();
            stocks.at(ItemType::SickPatient) -= nbPatientsTransferred;
            sickMutex.unlock();

            insurance->invoice(nbPatientsTransferred * getCostPerService(ServiceType::Transport), this);
            nbEmployeesPaid++;
        }
        else moneyMutex.unlock();
    }
}

void Ambulance::pay(int bill) {
    moneyMutex.lock();
    money += bill;
    moneyMutex.unlock();
}

void Ambulance::setHospitals(std::vector<Seller *> h) { hospitals = std::move(h); } // à appeller dans hospital.cpp?

void Ambulance::setInsurance(Seller *ins) { insurance = ins; } // à appeler dans insurance.cpp?

int Ambulance::getNumberPatients() { return stocks[ItemType::SickPatient]; }
