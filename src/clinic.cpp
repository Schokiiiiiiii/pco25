#include "clinic.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>
#include <iostream>
#include <random>

#include "hospital.h"

Clinic::Clinic(int id, int fund, std::vector<ItemType> resourcesNeeded)
: Seller(fund, id), resourcesNeeded(std::move(resourcesNeeded)) {
    for (auto it : this->resourcesNeeded) {
        stocks[it] = 0;
    }

    stocks[ItemType::SickPatient] = 0;
    stocks[ItemType::RehabPatient] = 0;
}

void Clinic::run() {
    logger() << "Clinic " <<  uniqueId << " starting with fund " << money << std::endl;

    while (true) {
        clock->worker_wait_day_start();
        if (PcoThread::thisThread()->stopRequested()) break;

        // Essayer de traiter le prochain patient
        processNextPatient();

        // Transférer les patients déjà traités vers un hôpital pour leur réhabilitation
        sendPatientsToRehab();

        // Payer les factures en retard
        payBills();

        clock->worker_end_day();
    }

    logger() << "Clinic " <<  uniqueId << " stopping with fund " << money << std::endl;
}


int Clinic::transfer(ItemType what, int qty) {

    // si on a pas de facture et de l'argent
    if (unpaidBills.empty() && money > 0) {

        // trouver combien on peut en accepter suivant le coût de traitement
        int endQty = qty;
        while (endQty * getCostPerService(ServiceType::Treatment) < 0) { --endQty; }

        // ajouter le nombre de nouveaux patients et le retourner
        queueSick += endQty;
        return endQty;
    }

    // sinon retourner 0 patient acceptés
    return 0;
}

bool Clinic::hasResourcesForTreatment() const {

    // tourner sur toutes les ressources vérifier qu'on en ait 1 de chaque
    bool hasResources = true;
    for (const auto& item : resourcesNeeded)
        if (stocks.at(item) < 1)
            hasResources = false;

    // retourner le résultat
    return hasResources;
}

bool Clinic::hasMoneyForTreatment() const {
    return (money - getEmployeeSalary(EmployeeType::TreatmentSpecialist)) >= 0;
}

void Clinic::payBills() {

    // tourner sur toutes les factures
    for (auto bill = unpaidBills.cbegin() ; bill != unpaidBills.cend() ; ++bill) {

        // si on peut la payer, la payer et l'effacer
        if (bill->second <= money) {
            bill->first->pay(bill->second);
            money -= bill->second;
            unpaidBills.erase(bill);
        }
    }
}

void Clinic::processNextPatient() {

    // vérifier qu'on a des patients
    if (stocks[ItemType::SickPatient] < 1)
        return;

    // commander le matériel nécessaire
    orderResources();

    // traiter le patient si on peut
    if (hasMoneyForTreatment() && hasResourcesForTreatment())
        treatOne();
}

void Clinic::sendPatientsToRehab() {

    // tourner sur tous les hôpitaux
    for (Seller* hospital : hospitals) {

        // si on a plus de patients arrêter
        if (!stocks[ItemType::RehabPatient]) break;

        // demander à l'hôpital et enregistrer le nombre de patients acceptés
        const int nbAccepted = hospital->transfer(ItemType::RehabPatient, stocks[ItemType::RehabPatient]);
        stocks[ItemType::RehabPatient] -= nbAccepted;
        invoice(nbAccepted * getCostPerService(ServiceType::Treatment), insurance);
    }
}

void Clinic::orderResources() {

    // si on a besoin de l'item
    for (const auto& item : resourcesNeeded) {
        if (stocks.at(item) < 1) {

            // demander à chaque seller pour une unité
            for (Seller* supplier : suppliers) {

                // arrêter si on nous en vend 1 et enregistrer la vente
                if (int price = supplier->buy(item, 1); price > 0) {
                    stocks.at(item) += 1;
                    unpaidBills.emplace_back(dynamic_cast<Supplier *>(supplier), price);
                    break;
                }
            }
        }
    }
}

void Clinic::treatOne() {

    // enlever un item de chaque
    for (ItemType item : resourcesNeeded)
        --stocks.at(item);

    // guérir le patient
    --stocks[ItemType::SickPatient];
    ++stocks[ItemType::RehabPatient];

    // payer le spécialiste
    money -= getEmployeeSalary(EmployeeType::TreatmentSpecialist);
}

void Clinic::pay(int bill) {
    money += bill;
}

Supplier *Clinic::chooseRandomSupplier(ItemType item) {
    std::vector<Supplier*> availableSuppliers;

    // Sélectionner les Suppliers qui ont la ressource recherchée
    for (Seller* seller : suppliers) {
        auto* sup = dynamic_cast<Supplier*>(seller);
        if (sup->sellsResource(item)) {
            availableSuppliers.push_back(sup);
        }
    }

    // Choisir aléatoirement un Supplier dans la liste
    assert(availableSuppliers.size());
    std::vector<Supplier*> out;
    std::sample(availableSuppliers.begin(), availableSuppliers.end(), std::back_inserter(out),
            1, std::mt19937{std::random_device{}()});
    return out.front();
}

void Clinic::setHospitalsAndSuppliers(std::vector<Seller*> hospitals, std::vector<Seller*> suppliers) {
    this->hospitals = hospitals;
    this->suppliers = suppliers;
}

void Clinic::setInsurance(Seller* ins) { 
    insurance = ins; 
}


int Clinic::getTreatmentCost() {
    return 0;
}

int Clinic::getWaitingPatients() {
    return stocks[ItemType::SickPatient];
}

int Clinic::getNumberPatients() {
    return stocks[ItemType::SickPatient] + stocks[ItemType::RehabPatient];
}

Pulmonology::Pulmonology(int uniqueId, int fund) :
    Clinic::Clinic(uniqueId, fund, {ItemType::Pill, ItemType::Thermometer}) {}

Cardiology::Cardiology(int uniqueId, int fund) :
    Clinic::Clinic(uniqueId, fund, {ItemType::Syringe, ItemType::Stethoscope}) {}

Neurology::Neurology(int uniqueId, int fund) :
    Clinic::Clinic(uniqueId, fund, {ItemType::Pill, ItemType::Scalpel}) {}
