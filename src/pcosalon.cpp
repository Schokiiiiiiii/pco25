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
}

PcoSalon::~PcoSalon() {
    delete seats;
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
    monitorIn();

    // if not enough space, return false
    if (nbClientsWaiting >= _nb_sieges) {
        monitorOut();
        return false;
    }

    // go inside the barber shop
    animationClientAccessEntrance(clientId);

    // check if barber is sleeping
    if (isBarberSleeping) { // wake him up
        signal(barberSleeping);
    } else { // wait in line (already a client)
        ++nbClientsWaiting;
        animationClientSitOnChair(clientId, findSeat());
        wait(clientWaiting);
        --nbClientsWaiting;
    }

    monitorOut();

    return true;
}

void PcoSalon::goForHairCut(unsigned clientId) {
    // done - Fabien
    monitorIn();

    // go to the working chair
    animationClientSitOnWorkChair(clientId);
    signal(barberWaitsAtChair);

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
    return nbClientsWaiting;
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
    assert(nbClientsWaiting > 0 && "Barber tried to pick a client despite no cients waiting");

    // wake up next client waiting
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

    monitorOut();

}

/********************************************
 *    Méthodes générales de l'interface     *
 *******************************************/
bool PcoSalon::isInService()
{
    // TODO
}


void PcoSalon::endService()
{
    // TODO
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
