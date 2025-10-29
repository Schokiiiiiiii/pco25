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

    // Tant qu'on peut produire de nouvelles ressources
    bool canProduce = true;
    while (canProduce) {

        // Choisir quel item a le moins de stock (recherche du minimum)
        ItemType lowestStockItem = resourcesSupplied.front();
        for (ItemType item : resourcesSupplied)
            if (stocks[item] < stocks[lowestStockItem])
                lowestStockItem = item;

        // Vérifier si après réduction, on est toujours en positif
        if (const int newMoney = (money - getEmployeeSalary(getEmployeeThatProduces(lowestStockItem)))) {
            money -= newMoney;
            ++stocks[lowestStockItem];
        } else {
            canProduce = false;
        }
    }
}

int Supplier::buy(ItemType it, int qty) {
    return std::min(stocks[it], qty);
}

void Supplier::pay(int bill) {
    money += bill;
}

int Supplier::getMaterialCost() {
    int totalCost = 0;
    for (const auto& item : resourcesSupplied) {
        totalCost += getCostPerUnit(item);
    }
    return totalCost;
}

bool Supplier::sellsResource(ItemType item) const {
    return std::find(resourcesSupplied.begin(), resourcesSupplied.end(), item) != resourcesSupplied.end();
}
