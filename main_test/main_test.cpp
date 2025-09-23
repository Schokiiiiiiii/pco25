#include <gtest/gtest.h>

#include "primenumberdetector.h"

// Test suite testing if numbers are prime
void testOne(PrimeNumberDetectorInterface& primeDetector)
{
    // Various prime numbers:
    // 433494437
    // 2971215073
    // 68720001023
    // 4398050705407
    // 70368760954879
    // 18014398241046527
    // 99194853094755497
    ASSERT_EQ(primeDetector.isPrime(433494437), true);
    ASSERT_EQ(primeDetector.isPrime(433494436), false);
    ASSERT_EQ(primeDetector.isPrime(2971215073), true);
    ASSERT_EQ(primeDetector.isPrime(68720001023), true);
    ASSERT_EQ(primeDetector.isPrime(4398050705407), true);
    ASSERT_EQ(primeDetector.isPrime(70368760954879), true);
    ASSERT_EQ(primeDetector.isPrime(18014398241046527), true);
    ASSERT_EQ(primeDetector.isPrime(99194853094755497), true);
    ASSERT_EQ(primeDetector.isPrime(18014398241046529), false);
    ASSERT_EQ(primeDetector.isPrime(99194853094755499), false);
}

TEST(Prime, SingleThreadedVersion) {
    PrimeNumberDetector detector;
    testOne(detector);
}

TEST(Prime, Test1Thread) {
    PrimeNumberDetectorMultiThread detector(1);
    testOne(detector);
}

TEST(Prime, Test2Threads) {
    PrimeNumberDetectorMultiThread detector(2);
    testOne(detector);
}

TEST(Prime, Test3Threads) {
    PrimeNumberDetectorMultiThread detector(3);
    testOne(detector);
}

TEST(Prime, Test12Threads) {
    PrimeNumberDetectorMultiThread detector(12);
    testOne(detector);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
