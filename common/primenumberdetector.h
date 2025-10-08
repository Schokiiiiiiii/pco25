#ifndef PRIMENUMBERDETECTOR_H
#define PRIMENUMBERDETECTOR_H

#include "logging.h"

#include <cstdint>
#include <cstddef>
#include <cmath>
#include <pcosynchro/pcothread.h>

class PrimeNumberDetectorInterface
{
public:
    virtual bool isPrime(uint64_t number) = 0;
};

class PrimeNumberDetector : public PrimeNumberDetectorInterface
{
public:
    bool isPrime(uint64_t /*number*/) override;
};

class PrimeNumberDetectorMultiThread : public PrimeNumberDetectorInterface
{
    size_t nbThreads;
    static void isPrimeRange(uint64_t number, uint64_t lower, uint64_t upper, bool* isPrime);
public:
    PrimeNumberDetectorMultiThread(size_t /*nbThreads*/);

    bool isPrime(uint64_t /*number*/) override;
};

#endif // PRIMENUMBERDETECTOR_H
