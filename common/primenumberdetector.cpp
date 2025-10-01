// Author(s) : Prénom Nom, Prénom Nom

#include "primenumberdetector.h"

bool PrimeNumberDetector::isPrime(uint64_t number) {
    auto upper_bound = static_cast<uint64_t>(sqrt(number)); //casting a double to int rounds it towards 0
    while (upper_bound > 1) { // the square root of the max 64 bits value is in the 10⁹ range
        if (!(number % upper_bound)) return false;
        --upper_bound;
    }
    return true;
}

PrimeNumberDetectorMultiThread::PrimeNumberDetectorMultiThread(size_t nbThreads) {
    // TODO
}

bool PrimeNumberDetectorMultiThread::isPrime(uint64_t number) {
    // TODO
    return false;
}
