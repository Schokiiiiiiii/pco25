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
#include <string>
#include <exception>

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

    // check there is a seat available at least
    if (nbClientsWaiting >= _nb_sieges)
        throw std::runtime_error("Cannot find a seat with no seats available");

    // look for a seat that is free
    for (int i = 0 ; i < _nb_sieges ; ++i)
        if (!seats[i])
            return i;

    // we should always find a seat
    assert(false && "findSeat() could not find a seat");
}

bool PcoSalon::accessSalon(unsigned clientId) {
    // done

    // go inside the barber shop
    animationClientAccessEntrance(clientId);

    monitorIn();

    // if not enough space, return false
    if (nbClientsWaiting >= _nb_sieges) {
        monitorOut();
        return false;
    }

    // check if barber is sleeping
    if (isBarberSleeping) {

        // wake barber up
        _interface->consoleAppendTextClient(clientId, "Réveilles-toi barbier...");
        isClientReady = true;
        signal(barberSleeping);

        // get out of monitor
        monitorOut();
        animationWakeUpBarber();
        return true;
    }

    // find a seat
    const int seat = findSeat();
    seats[seat] = true;

    // wait on the seat
    ++nbClientsWaiting;
    _interface->consoleAppendTextClient(clientId, ("J'attends sur la chaise no " + std::to_string(seat)).data());
    animationClientSitOnChair(clientId, seat);
    wait(clientWaiting);

    // finished waiting
    --nbClientsWaiting;
    seats[seat] = false;

    monitorOut();

    return true;
}

void PcoSalon::goForHairCut(unsigned clientId) {
    // done
    monitorIn();

    // go to the working chair
    _interface->consoleAppendTextClient(clientId, "J'ai attendu longtemps !");
    animationClientSitOnWorkChair(clientId);
    isClientReady = false;
    isClientOnChair = true;
    signal(barberWaitsAtChair);

    // wait for barber to finish
    if (!haircutDone)
        wait(clientBeautifying);

    // put flags back to normal
    _interface->consoleAppendTextClient(clientId, "Superbe coupe chef.");
    isClientOnChair = false;
    haircutDone = false;

    monitorOut();
}

void PcoSalon::waitingForHairToGrow(unsigned clientId) {
    // done
    monitorIn();

    // wait for hait to grow
    animationClientWaitForHairToGrow(clientId);

    monitorOut();
}


void PcoSalon::walkAround(unsigned clientId) {
    // done
    monitorIn();

    // walk around
    animationClientWalkAround(clientId);

    monitorOut();
}


void PcoSalon::goHome(unsigned clientId) {
    // done
    monitorIn();

    // go home
    animationClientGoHome(clientId);

    monitorOut();
}


/********************************************
 * Méthodes de l'interface pour le barbier  *
 *******************************************/
unsigned int PcoSalon::getNbClient() {
    // done
    monitorIn();
    const unsigned int nb = nbClientsWaiting + isClientReady + isClientOnChair;
    monitorOut();
    return nb;
}

void PcoSalon::goToSleep() {
    // done
    monitorIn();

    // go to sleep
    isBarberSleeping = true;
    animationBarberGoToSleep();
    if (isBarberSleeping)
        wait(barberSleeping);
    isBarberSleeping = false;

    monitorOut();
}


void PcoSalon::pickNextClient() {
    // done
    monitorIn();

    // there should always be clients waiting when calling pickNextClient()
    assert((isClientReady || nbClientsWaiting > 0) && "Barber tried to pick a client despite no clients waiting");

    // only signal waiting client if not already one waiting
    if (!isClientReady)
        signal(clientWaiting);

    monitorOut();
}


void PcoSalon::waitClientAtChair() {
    // done
    monitorIn();

    // wait if client is not on chair yet
    if (!isClientOnChair)
        wait(barberWaitsAtChair);

    monitorOut();
}


void PcoSalon::beautifyClient() {
    // done
    monitorIn();

    // BEAUTIFY
    _interface->consoleAppendTextBarber("On va vous faire une belle coupe.");
    animationBarberCuttingHair();

    // signal client haircut is done
    haircutDone = true;
    signal(clientBeautifying);
    _interface->consoleAppendTextBarber("Cela coûtera 50.- CHF !");

    monitorOut();

}

/********************************************
 *    Méthodes générales de l'interface     *
 *******************************************/
bool PcoSalon::isInService() {
    // done
    monitorIn();
    const bool Service = isSalonInService;
    monitorOut();
    return Service;
}


void PcoSalon::endService() {
    // done
    monitorIn();

    // end service
    isSalonInService = false;

    // we only want to unstuck barber, clients will still get a haircut
    signal(barberSleeping);
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
