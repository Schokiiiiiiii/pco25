/*  _____   _____ ____    ___   ___ ___  ____
 * |  __ \ / ____/ __ \  |__ \ / _ \__ \| ____|
 * | |__) | |   | |  | |    ) | | | | ) || |___
 * |  ___/| |   | |  | |   / /| | | |/ / |____|
 * | |    | |___| |__| |  / /_| |_| / /_  ____|
 * |_|     \_____\____/  |____|\___/____||_____|
 */
// Modifications à faire dans le fichier

#include "barber.h"
#include <unistd.h>

#include <iostream>

Barber::Barber(GraphicSalonInterface *interface, std::shared_ptr<SalonBarberInterface> salon)
    : _interface(interface), _salon(salon) {

    _interface->consoleAppendTextBarber("Salut, prêt à travailler !");
}

void Barber::run() {

    // modified - Fabien

    // we keep looping until closed
    while (_salon->isInService()) {

        // check if there are any clients
        if (_salon->getNbClient()) {

            // pick the next client
            _interface->barberPicksNewClient();
            _salon->pickNextClient();
        } else {

            // go to sleep
            _interface->barberGoToSleep();
            _salon->goToSleep();
            _interface->barberStopSleeping();
        }

        // wait client at the working chair
        _interface->barberGoesHairCut();
        _salon->waitClientAtChair();

        // cut client's hair
        _interface->barberCuttingHair();
        _salon->beautifyClient();
    }

    _interface->consoleAppendTextBarber("La journée est terminée, à demain !");
}
