/*  _____   _____ ____    ___   ___ ___  ____
 * |  __ \ / ____/ __ \  |__ \ / _ \__ \| ____|
 * | |__) | |   | |  | |    ) | | | | ) || |___
 * |  ___/| |   | |  | |   / /| | | |/ / |____|
 * | |    | |___| |__| |  / /_| |_| / /_  ____|
 * |_|     \_____\____/  |____|\___/____||_____|
 */
// Modifications à faire dans le fichier

#include "client.h"
#include <unistd.h>

#include <iostream>

int Client::_nextId = 0;

Client::Client(GraphicSalonInterface *interface, std::shared_ptr<SalonClientInterface> salon)

    : _interface(interface), _salon(salon),  _clientId(_nextId++)
{
    _interface->consoleAppendTextClient(_clientId, "Salut, prêt pour une coupe !");
}

void Client::run() {
    // done

    // try to access salon
    while (_salon->isInService()) {
        if(_salon->accessSalon(_clientId)) { // access salon

            // get a haircut
            _salon->goForHairCut(_clientId);

            // wait for hair to grow
            _salon->waitingForHairToGrow(_clientId);

        } else { // walk around

            // walk around
            _interface->consoleAppendTextClient(_clientId, "Le salon est plein... Je vais faire un tour !");
            _salon->walkAround(_clientId);
        }
    }

    _salon->goHome(_clientId);
    _interface->consoleAppendTextClient(_clientId, "Le salon est fermé... Zut !");
}