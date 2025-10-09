// Author(s) : Prénom Nom, Prénom Nom

#include "primenumberdetector.h"
#include <iostream>

bool PrimeNumberDetector::isPrime(uint64_t number) {
    auto upper_bound = static_cast<uint64_t>(sqrt(number)); //casting a double to int rounds it towards 0
    while (upper_bound > 1) { // the square root of the max 64 bits value is in the 10⁹ range
        if (!(number % upper_bound)) return false;
        --upper_bound;
    }
    return true;
}

// POUR COMPILER LE PROJET FAIS LE DANS LA LIGNE DE COMMANDE:
// cmake -B build
// cmake --build build

PrimeNumberDetectorMultiThread::PrimeNumberDetectorMultiThread(size_t nbThreads) {

    if (nbThreads < 1) throw std::invalid_argument("Invalid number of threads!");

    else threads_count = nbThreads;
}

bool PrimeNumberDetectorMultiThread::isPrime(uint64_t number) {

    return false;
}
