//  /$$$$$$$   /$$$$$$   /$$$$$$         /$$$$$$   /$$$$$$   /$$$$$$  /$$$$$$$ 
// | $$__  $$ /$$__  $$ /$$__  $$       /$$__  $$ /$$$_  $$ /$$__  $$| $$____/ 
// | $$  \ $$| $$  \__/| $$  \ $$      |__/  \ $$| $$$$\ $$|__/  \ $$| $$      
// | $$$$$$$/| $$      | $$  | $$        /$$$$$$/| $$ $$ $$  /$$$$$$/| $$$$$$$ 
// | $$____/ | $$      | $$  | $$       /$$____/ | $$\ $$$$ /$$____/ |_____  $$
// | $$      | $$    $$| $$  | $$      | $$      | $$ \ $$$| $$       /$$  \ $$
// | $$      |  $$$$$$/|  $$$$$$/      | $$$$$$$$|  $$$$$$/| $$$$$$$$|  $$$$$$/
// |__/       \______/  \______/       |________/ \______/ |________/ \______/ 

#include <gtest/gtest.h>
#include <atomic>
#include <vector>

#include <pcosynchro/pcothread.h>
#include <pcosynchro/pcosemaphore.h>

#include "sharedsection.h"
#include "sharedsectioninterface.h"

static void enterCritical(std::atomic<int>& nbIn) {
    int now = nbIn.fetch_add(1) + 1;
    ASSERT_EQ(now, 1) << "Deux locomotives dans la section en même temps !";
}
static void leaveCritical(std::atomic<int>& nbIn) {
    nbIn.fetch_sub(1);
}

TEST(SharedSection, TwoSameDirection_SerializesCorrectly) {
    SharedSection section;
    std::atomic<int> nbIn{0};
    Locomotive l1(1, 10, 0), l2(2, 10, 0);

    PcoThread t1([&]{
        section.access(l1, SharedSectionInterface::Direction::D1);
        enterCritical(nbIn);
        PcoThread::usleep(1000);
        leaveCritical(nbIn);
        section.leave(l1, SharedSectionInterface::Direction::D1);
        section.release(l1);
    });

    PcoThread t2([&]{
        PcoThread::usleep(500);
        section.access(l2, SharedSectionInterface::Direction::D1);
        enterCritical(nbIn);
        leaveCritical(nbIn);
        section.leave(l2, SharedSectionInterface::Direction::D1);
    });

    t1.join(); t2.join();
    ASSERT_EQ(section.nbErrors(), 0);
}

TEST(SharedSection, ConsecutiveAccess_IsError) {
    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.access(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D1);

    ASSERT_EQ(section.nbErrors(), 1);
}

TEST(SharedSection, LeaveWrongDirection_IsError) {
    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D2); 

    ASSERT_EQ(section.nbErrors(), 1);
}

/***************************************
 ************** NEW TESTS **************
 ***************************************/

TEST(SharedSectionStudent, ConsecutiveLeave_IsError) {

    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D1);

    ASSERT_EQ(section.nbErrors(), 1);
}

TEST(SharedSectionStudent, ConsecutiveRelease_IsError) {

    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D1);
    section.release(l1);
    section.release(l1);

    ASSERT_EQ(section.nbErrors(), 1);
}

TEST(SharedSectionStudent, LeaveWithoutAccess_IsError) {

    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.leave(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D2);

    ASSERT_EQ(section.nbErrors(), 2);
}

TEST(SharedSectionStudent, ReleaseWithoutAccess_IsError) {

    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.release(l1);

    ASSERT_EQ(section.nbErrors(), 1);
}

TEST(SharedSectionStudent, ReleaseWithoutLeave_IsError) {

    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.release(l1);

    ASSERT_EQ(section.nbErrors(), 1);
}

TEST(SharedSectionStudent, MultipleErrors_CountCorrectly) {

    SharedSection section;
    Locomotive l1(1, 10, 0);

    const int nbErrors = 5;

    for (int i = 0 ; i < nbErrors ; ++i)
        section.release(l1);

    ASSERT_EQ(section.nbErrors(), nbErrors);
}

TEST(SharedSectionStudent, GoThroughSharedSectionMultipleTimes_IsNoErrors) {
    SharedSection section;
    Locomotive l1(1, 10, 0);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D1);
    section.release(l1);

    section.access(l1, SharedSectionInterface::Direction::D2);
    section.leave(l1, SharedSectionInterface::Direction::D2);
    section.release(l1);

    section.access(l1, SharedSectionInterface::Direction::D2);
    section.leave(l1, SharedSectionInterface::Direction::D2);
    section.release(l1);

    ASSERT_EQ(section.nbErrors(), 0);
}

TEST(TwoTrainsInteractions, TwoOppositeDirection_SerializesCorrectly) {
    SharedSection section;
    std::atomic<int> nbIn{0};
    Locomotive l1(1, 10, 0), l2(2, 10, 0);

    PcoThread t1([&]{
        section.access(l1, SharedSectionInterface::Direction::D1);
        enterCritical(nbIn);
        PcoThread::usleep(1000);
        leaveCritical(nbIn);
        section.leave(l1, SharedSectionInterface::Direction::D1);
        section.release(l1);
    });

    PcoThread t2([&]{
        PcoThread::usleep(500);
        section.access(l2, SharedSectionInterface::Direction::D2);
        enterCritical(nbIn);
        leaveCritical(nbIn);
        section.leave(l2, SharedSectionInterface::Direction::D2);
    });

    t1.join(); t2.join();
    ASSERT_EQ(section.nbErrors(), 0);
}

TEST(TwoTrainsInteractions, SecondLocoLeaveOrReleaseWithoutAccess_IsError) {
    SharedSection section;
    std::atomic<int> nbIn{0};
    Locomotive l1(1, 10, 0), l2(2, 10, 0);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.leave(l2, SharedSectionInterface::Direction::D1);
    section.leave(l2, SharedSectionInterface::Direction::D2);
    section.release(l2);

    ASSERT_EQ(section.nbErrors(), 3);
}

TEST(TwoTrainsInteractions, SecondLocoReleaseBeforeFirstLocoRelease_IsError) {
    SharedSection section;
    std::atomic<int> nbIn{0};
    Locomotive l1(1, 10, 0), l2(2, 10, 0);

    section.access(l1, SharedSectionInterface::Direction::D1);
    section.leave(l1, SharedSectionInterface::Direction::D1);
    section.release(l2);

    ASSERT_EQ(section.nbErrors(), 1);
}

TEST(ThreeTrainsInteractions, ThreeTrains_SerializesCorrectly) {
    SharedSection section;
    std::atomic<int> nbIn{0};
    Locomotive l1(1, 10, 0), l2(2, 10, 0), l3(3, 10, 0);

    PcoThread t1([&]{
        section.access(l1, SharedSectionInterface::Direction::D1);
        enterCritical(nbIn);
        PcoThread::usleep(1500);
        leaveCritical(nbIn);
        section.leave(l1, SharedSectionInterface::Direction::D1);
        section.release(l1);
    });

    PcoThread t2([&]{
        PcoThread::usleep(1000);
        section.access(l2, SharedSectionInterface::Direction::D2);
        enterCritical(nbIn);
        leaveCritical(nbIn);
        section.leave(l2, SharedSectionInterface::Direction::D2);
        section.release(l2);
    });

    PcoThread t3([&]{
        PcoThread::usleep(500);
        section.access(l3, SharedSectionInterface::Direction::D2);
        enterCritical(nbIn);
        leaveCritical(nbIn);
        section.leave(l3, SharedSectionInterface::Direction::D2);
        section.release(l3);
    });

    t1.join(); t2.join(); t3.join();
    ASSERT_EQ(section.nbErrors(), 0);
}