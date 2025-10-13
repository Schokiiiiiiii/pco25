#ifndef PRIMENUMBERDETECTOR_H
#define PRIMENUMBERDETECTOR_H

#include "logging.h"

#include <cstdint>
#include <cstddef>
#include <cmath>
#include <pcosynchro/pcothread.h>
#include <vector>

// we check every Nth number if we have found a divisor
const static int N = 5;

class PrimeNumberDetectorInterface {
public:
    virtual bool isPrime(uint64_t number) = 0;
};

class PrimeNumberDetector : public PrimeNumberDetectorInterface {
public:
    bool isPrime(uint64_t /*number*/) override;
};

class PrimeNumberDetectorMultiThread : public PrimeNumberDetectorInterface {
    size_t nbThreads;
    static void isPrimeRange(uint64_t number, uint64_t lower, uint64_t upper, bool volatile *isPrime);
public:
    PrimeNumberDetectorMultiThread(size_t /*nbThreads*/);

    bool isPrime(uint64_t /*number*/) override;
};

#endif // PRIMENUMBERDETECTOR_H
