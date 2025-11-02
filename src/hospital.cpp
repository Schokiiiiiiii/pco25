// hospital.cpp
#include "hospital.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>

Hospital::Hospital(int id, int fund, int maxBeds)
: Seller(fund, id), maxBeds(maxBeds), nbNursingStaff(maxBeds) {
    stocks[ItemType::SickPatient] = 0;
    stocks[ItemType::RehabPatient] = 0;
}

void Hospital::run() {
    logger() << "Hospital " <<  uniqueId << " starting with fund " << money << ", maxBeds " << maxBeds << std::endl;

    while (true) {
        clock->worker_wait_day_start();
        if (PcoThread::thisThread()->stopRequested()) break;

        transferSickPatientsToClinic();
        updateRehab();
        payNursingStaff();

        clock->worker_end_day();
    }

    logger() << "Hospital " <<  uniqueId << " stopping with fund " << money << std::endl;
}

void Hospital::transferSickPatientsToClinic() {

    // TODO

}

void Hospital::updateRehab() {

    // TODO

}

void Hospital::payNursingStaff() {

    moneyMutex.lock();
    money -= getEmployeeSalary(EmployeeType::NursingStaff) * nbNursingStaff;
    moneyMutex.unlock();

    nbEmployeesPaid += nbNursingStaff;

}

void Hospital::pay(int bill) {
    moneyMutex.lock();
    money += bill;
    moneyMutex.unlock();
}

int Hospital::transfer(ItemType what, int qty) {
    
    // TODO
    // transfer fait le transfert de patients depuis les ambulances, pour autant qu'ill y ait de la place dans l'hopital, et que money soit plus grand que 0
    // il fait  ausssi le transfert depuis les cliniques, auquel cas le Itemtype deevient brehapatient

}

int Hospital::getNumberPatients() {
    return stocks[ItemType::SickPatient] + stocks[ItemType::RehabPatient] + nbFreed;
}

void Hospital::setClinics(std::vector<Seller*> c) {
    clinics = std::move(c);
}

void Hospital::setInsurance(Seller* ins) { 
    insurance = ins; 
}
