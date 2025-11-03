// hospital.cpp
#include "hospital.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>

static int rehabPatientsYesterday = 0;

Hospital::Hospital(int id, int fund, int maxBeds)
: Seller(fund, id), maxBeds(maxBeds), nbNursingStaff(maxBeds) { // le nombre de staff est égal au nombre max de lits
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
    // y a pas une condition sur le nombre transféré vers les cliniques?
    auto *clinic = chooseRandomSeller(clinics);

    int sickTransfered = clinic->transfer(ItemType::SickPatient, stocks[ItemType::SickPatient]);

    sickMutex.lock();
    stocks[ItemType::SickPatient] -= sickTransfered;
    sickMutex.unlock();

    insurance->invoice(sickTransfered * getCostPerService(ServiceType::PreTreatmentStay), this);
}

void Hospital::updateRehab() {

    int index = clock->current_day() % 5; // séjour de convalescence 5 jours, inclus ou exclus?
    int rehabTransfered = rehabSchedule[index];

    // section critique
    rehabMutex.lock();
    rehabSchedule[index] = (stocks[ItemType::RehabPatient] - rehabPatientsYesterday > 0 ? stocks[ItemType::RehabPatient] - rehabPatientsYesterday : 0);
    rehabPatientsYesterday = stocks[ItemType::RehabPatient];

    stocks[ItemType::RehabPatient] -= rehabTransfered;
    rehabMutex.unlock();
    // fin section critique

    insurance->invoice(rehabTransfered * getCostPerService(ServiceType::Rehab), this);
}

void Hospital::payNursingStaff() {
    // the staff gets paid in any case
    moneyMutex.lock();
    money -= getEmployeeSalary(EmployeeType::NursingStaff) * nbNursingStaff;
    moneyMutex.unlock();

    nbEmployeesPaid += nbNursingStaff;
}

// hospital gets paid
void Hospital::pay(int bill) {
    moneyMutex.lock();
    money += bill;
    moneyMutex.unlock();
}

// transfer fait le transfert de patients depuis les ambulances, pour autant qu'il y ait de la place dans l'hopital, et que money ne soit pas négatif
// il fait aussi le transfert depuis les cliniques, auquel cas le Itemtype devient rehabPatient
int Hospital::transfer(ItemType what, int qty) {

    int freeBeds = maxBeds - stocks[ItemType::SickPatient] - stocks[ItemType::RehabPatient];
    int isAdded = (freeBeds > qty ? qty : freeBeds);

    // dans l'enum class itemtype, sickpatient = 0 et rehabpatient = 1
    if (money >= 0) {
        switch (what) {
        case ItemType::SickPatient:
            sickMutex.lock();
            stocks[what] += isAdded;
            sickMutex.unlock();
            return isAdded;
        case ItemType::RehabPatient:
            rehabMutex.lock();
            stocks[what] += isAdded;
            rehabMutex.unlock();
            return isAdded;
        default:
            break;
        }
    }
    return 0; // impossible de transférer autre choses que des patients
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
