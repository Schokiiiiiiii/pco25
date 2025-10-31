#include "supplier.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>
#include <iostream>


Supplier::Supplier(int uniqueId, int fund, std::vector<ItemType> resourcesSupplied)
    : Seller(fund, uniqueId), resourcesSupplied(resourcesSupplied) {
    for (const auto& item : resourcesSupplied) {    
        stocks[item] = 0;    
    }
}

void Supplier::run() {
    logger() << "Supplier " <<  uniqueId << " starting with fund " << money << std::endl;

    while (true) {
        clock->worker_wait_day_start();
        if (PcoThread::thisThread()->stopRequested()) break;

        attemptToProduceResource();

        clock->worker_end_day();
    }

    logger() << "Supplier " <<  uniqueId << " stopping with fund " << money << std::endl;
}

void Supplier::attemptToProduceResource() {

    // trouver un item random
    // (chooseRandomItem demande une map sauf que resourcesSupplied est un vecteur du coup faut le faire manuellement)
    const ItemType item = resourcesSupplied.at(rand() % resourcesSupplied.size());

    // vérifier si après réduction, on est toujours en positif
    if (const int newMoney = (money - getEmployeeSalary(getEmployeeThatProduces(item))); newMoney >= 0) {

        // payer l'employer et ajouter la ressource au stock
        money = newMoney;
        ++stocks.at(item);
    }
}

int Supplier::buy(ItemType it, int qty) {
    if (qty > stocks.at(it))
        return 0;
    return qty * getCostPerUnit(it);
}

void Supplier::pay(int bill) {
    money += bill;
}

int Supplier::getMaterialCost() const {
    int totalCost = 0;
    for (const auto& item : resourcesSupplied) {
        totalCost += getCostPerUnit(item);
    }
    return totalCost;
}

bool Supplier::sellsResource(ItemType item) const {
    return std::find(resourcesSupplied.begin(), resourcesSupplied.end(), item) != resourcesSupplied.end();
}
