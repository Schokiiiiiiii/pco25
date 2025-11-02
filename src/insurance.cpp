#include "insurance.h"
#include "costs.h"
#include <pcosynchro/pcothread.h>

Insurance::Insurance(int uniqueId, int fund) : Seller(fund, uniqueId) {}

void Insurance::run() {
    logger() << "Insurance " <<  uniqueId << " starting with fund " << money << std::endl;

    while (true) {
        clock->worker_wait_day_start();
        if (PcoThread::thisThread()->stopRequested()) break;

        // Réception de la somme des cotisations journalières des assurés
        receiveContributions();

        // Payer les factures
        payBills();

        clock->worker_end_day();
    }

    logger() << "Insurance " <<  uniqueId << " stopping with fund " << money << std::endl;
}

void Insurance::receiveContributions() {
    money += INSURANCE_CONTRIBUTION;
}

void Insurance::invoice(int bill, Seller* who) {
    billMutex.lock();
    unpaidBills.emplace_back(who, bill);
    billMutex.unlock();
}

void Insurance::payBills() {

    billMutex.lock();;
    auto bill = unpaidBills.begin();
    while (bill != unpaidBills.end()) {

        if (bill->second <= money) {
            money -= bill->second;
            bill->first->pay(bill->second);
            bill = unpaidBills.erase(bill);
        } else {
            ++bill;
        }
    }
    billMutex.unlock();
}
