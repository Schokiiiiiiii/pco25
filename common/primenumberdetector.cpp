// Author(s) : Fabien Léger, Aymeric Bonny

#include "primenumberdetector.h"

#include <vector>

bool PrimeNumberDetector::isPrime(uint64_t number) {
    auto upper_bound = static_cast<uint64_t>(sqrt(number)); //casting a double to int rounds it towards 0
    while (upper_bound > 1) { // the square root of the max 64 bits value is in the 10⁹ range
        if (!(number % upper_bound)) return false;
        --upper_bound;
    }
    return true;
}

PrimeNumberDetectorMultiThread::PrimeNumberDetectorMultiThread(size_t nbThreads) {
    if (nbThreads < 1) throw std::invalid_argument("Invalid number of threads!");
    this -> nbThreads = nbThreads;
}

bool PrimeNumberDetectorMultiThread::isPrime(uint64_t number) {

    // initialize thread vector & result
    std::vector<PcoThread *> threads;
    bool volatile isPrime = true;

    // calculate square root of number
    auto sqrt_num = static_cast<uint64_t>(sqrt(number));

    // find range per thread and start
    auto size_range = static_cast<uint64_t>(sqrt_num / nbThreads);
    uint64_t start = 2;

    // initialize all threads
    for (size_t i = 0; i < nbThreads - 1; ++i) {
        threads.push_back(new PcoThread(isPrimeRange, number, start, start + size_range, &isPrime));
        start += size_range;
    }
    // initialize last thread
    threads.push_back(new PcoThread(isPrimeRange, number, start, sqrt_num, &isPrime));

    // wait for all threads to join
    for (size_t i = 0; i < nbThreads; ++i) {
        threads[i] -> join();
    }

    // delete all threads
    for (size_t i = 0; i < nbThreads; ++i) {
        delete threads[i];
    }

    // return result
    return isPrime;
}

void PrimeNumberDetectorMultiThread::isPrimeRange(const uint64_t number,
                                                const uint64_t lower,
                                                uint64_t upper,
                                                bool volatile *isPrime)
{
    while (upper >= lower) {
        if (!(number % upper)) *isPrime = false;

        // stops the thread if a divisor is found. It checks for this information every Nth number
        if (!(upper % N) && (*isPrime == false)) return;

        --upper;
    }
}
