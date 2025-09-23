#include "primenumberdetector.h"

#include <chrono>

void printElapsedTime(const std::chrono::steady_clock::time_point& begin,
                      const std::chrono::steady_clock::time_point& end);

int main() {
    // Various prime numbers:
    // 433494437
    // 2971215073
    // 68720001023
    // 4398050705407
    // 70368760954879
    // 18014398241046527
    // 99194853094755497
    uint64_t number = 99194853094755497;
    size_t nthreads = 2;

    // The prime number detectors
    PrimeNumberDetector pnd;
    PrimeNumberDetectorMultiThread pndm(nthreads);

    // Get time
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    // Launch single threaded version
    pnd.isPrime(number);

    // Get time
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    // Print elapsed time
    std::cout << "Single threaded version" << std::endl;
    printElapsedTime(begin, end);

    // Get time
    begin = std::chrono::steady_clock::now();

    // Launch multi threaded version
    pndm.isPrime(number);

    // Get time
    end = std::chrono::steady_clock::now();

    // Print elapsed time
    std::cout << "Multi threaded (" << nthreads << "t) version" << std::endl;
    printElapsedTime(begin, end);

    return 0;
}

void printElapsedTime(const std::chrono::steady_clock::time_point& begin,
                      const std::chrono::steady_clock::time_point& end) {
    std::cout << "Time elapsed = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms] "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[us] "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() << "[ns]" << std::endl;
}
