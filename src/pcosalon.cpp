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
    : _interface(interface)
{
    // TODO
}

/********************************************
 * Méthodes de l'interface pour les clients *
 *******************************************/
bool PcoSalon::accessSalon(unsigned clientId)
{
    // TODO
}


void PcoSalon::goForHairCut(unsigned clientId)
{
    // TODO
}

void PcoSalon::waitingForHairToGrow(unsigned clientId)
{
    // TODO
}


void PcoSalon::walkAround(unsigned clientId)
{
    // TODO
}


void PcoSalon::goHome(unsigned clientId){
    // TODO
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
    animationBarberGoToSleep();
    wait(barberSleeping);

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
    if (!clientOnChair)
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
