/*  _____   _____ ____    ___   ___ ___  ____
 * |  __ \ / ____/ __ \  |__ \ / _ \__ \| ____|
 * | |__) | |   | |  | |    ) | | | | ) || |___
 * |  ___/| |   | |  | |   / /| | | |/ / |____|
 * | |    | |___| |__| |  / /_| |_| / /_  ____|
 * |_|     \_____\____/  |____|\___/____||_____|
 */
// Modifications à faire dans le fichier

#include "pcosalon.h"

#include <pcosynchro/pcothread.h>

#include <iostream>

PcoSalon::PcoSalon(GraphicSalonInterface *interface, unsigned int capacity)
    : _interface(interface), _nb_sieges(capacity) {
    seats = new bool[capacity];
    for (size_t i = 0; i < capacity; ++i)
        seats[i] = false;
}

PcoSalon::~PcoSalon() {
    delete[] seats;
}

/********************************************
 * Méthodes de l'interface pour les clients *
 *******************************************/
int PcoSalon::findSeat() const {

    for (int i = 0 ; i < _nb_sieges ; ++i)
        if (!seats[i])
            return i;

    assert(false && "findSeat() could not find a seat");
}

bool PcoSalon::accessSalon(unsigned clientId) {

    // done - Fabien

    // go inside the barber shop
    animationClientAccessEntrance(clientId);

    monitorIn();

    // if not enough space, return false
    if (nbClientsWaiting >= _nb_sieges) {
        monitorOut();
        return false;
    }

    // check if barber is sleeping
    if (isBarberSleeping) { // wake him up
        isClientReady = true;
        signal(barberSleeping);
    } else { // wait in line (already a client)
        // sit down at a chair
        ++nbClientsWaiting;
        const int seat = findSeat();
        seats[seat] = true;
        animationClientSitOnChair(clientId, seat);
        wait(clientWaiting);

        // get woken up
        --nbClientsWaiting;
        seats[seat] = false;
    }

    monitorOut();

    return true;
}

void PcoSalon::goForHairCut(unsigned clientId) {
    // done - Fabien
    monitorIn();

    // go to the working chair
    animationClientSitOnWorkChair(clientId);
    isClientReady = false;
    isClientOnChair = true;
    signal(barberWaitsAtChair);

    // wait for barber to finish
    wait(clientBeautifying);

    isClientOnChair = false;

    monitorOut();
}

void PcoSalon::waitingForHairToGrow(unsigned clientId) {
    // done - Fabien
    monitorIn();

    // wait for hait to grow
    animationClientWaitForHairToGrow(clientId);

    monitorOut();
}


void PcoSalon::walkAround(unsigned clientId) {
    // done - Fabien
    monitorIn();

    // walk around
    animationClientWalkAround(clientId);

    monitorOut();
}


void PcoSalon::goHome(unsigned clientId) {
    // done - Fabien
    monitorIn();

    // go home
    animationClientGoHome(clientId);

    monitorOut();
}


/********************************************
 * Méthodes de l'interface pour le barbier  *
 *******************************************/
unsigned int PcoSalon::getNbClient() {
    // done - Fabien
    monitorIn();
    const unsigned int nb = nbClientsWaiting + isClientReady + isClientOnChair;
    monitorOut();
    return nb;
}

void PcoSalon::goToSleep() {
    // done - Fabien
    monitorIn();

    // go to sleep
    isBarberSleeping = true;
    animationBarberGoToSleep();
    wait(barberSleeping);
    isBarberSleeping = false;

    monitorOut();
}


void PcoSalon::pickNextClient() {
    // done - Fabien
    monitorIn();

    // there should always be clients waiting when calling pickNextClient()
    assert(nbClientsWaiting > 0 && "Barber tried to pick a client despite no clients waiting");

    // wake up next client waiting
    if (!isClientReady)
        signal(clientWaiting);

    monitorOut();
}


void PcoSalon::waitClientAtChair() {
    // done - Fabien
    monitorIn();

    // wait if client is not on chair yet
    if (!isClientOnChair)
        wait(barberWaitsAtChair);

    monitorOut();
}


void PcoSalon::beautifyClient() {
    // done - Fabien
    monitorIn();

    // BEAUTIFY
    animationBarberCuttingHair();

    signal(clientBeautifying);

    monitorOut();

}

/********************************************
 *    Méthodes générales de l'interface     *
 *******************************************/
bool PcoSalon::isInService() {
    // done Fabien
    monitorIn();
    const bool Service = isSalonInService;
    monitorOut();
    return Service;
}


void PcoSalon::endService() {
    // done - Fabien
    monitorIn();

    // end service
    isSalonInService = false;

    // wake up barber if sleeping
    signal(barberSleeping);

    // wake up barber if waiting
    signal(barberWaitsAtChair);

    monitorOut();
}

/********************************************
 *   Méthodes privées pour les animations   *
 *******************************************/

template<typename F>
void PcoSalon::animationCall(F&& f)
{
#if PCO_USE_HOARE_MONITOR
    monitorOut();
    f();
    monitorIn();
#else
    _mutex.unlock();
    f();
    _mutex.lock();
#endif
}

void PcoSalon::animationClientAccessEntrance(unsigned clientId)
{
    animationCall([&]{ _interface->clientAccessEntrance(clientId); });
}

void PcoSalon::animationClientSitOnChair(unsigned clientId, unsigned clientSitNb)
{
    animationCall([&]{ _interface->clientSitOnChair(clientId, clientSitNb);} );
}

void PcoSalon::animationClientSitOnWorkChair(unsigned clientId)
{
    animationCall([&]{_interface->clientSitOnWorkChair(clientId);} );
}

void PcoSalon::animationClientWaitForHairToGrow(unsigned clientId)
{
    animationCall([&]{_interface->clientWaitHairToGrow(clientId, true);} );
}

void PcoSalon::animationClientWalkAround(unsigned clientId)
{
    animationCall([&]{_interface->clientWalkAround(clientId);} );
}

void PcoSalon::animationBarberGoToSleep()
{
    animationCall([&]{_interface->barberGoToSleep();} );
}

void PcoSalon::animationWakeUpBarber()
{
    animationCall([&]{_interface->clientWakeUpBarber();} );
}

void PcoSalon::animationBarberCuttingHair()
{
    animationCall([&]{_interface->barberCuttingHair();});
}

void PcoSalon::animationClientGoHome(unsigned clientId){
    animationCall([&]{_interface->clientWaitHairToGrow(clientId, false);});
}
