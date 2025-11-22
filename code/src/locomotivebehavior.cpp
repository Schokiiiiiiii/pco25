//  /$$$$$$$   /$$$$$$   /$$$$$$         /$$$$$$   /$$$$$$   /$$$$$$  /$$$$$$$ 
// | $$__  $$ /$$__  $$ /$$__  $$       /$$__  $$ /$$$_  $$ /$$__  $$| $$____/ 
// | $$  \ $$| $$  \__/| $$  \ $$      |__/  \ $$| $$$$\ $$|__/  \ $$| $$      
// | $$$$$$$/| $$      | $$  | $$        /$$$$$$/| $$ $$ $$  /$$$$$$/| $$$$$$$ 
// | $$____/ | $$      | $$  | $$       /$$____/ | $$\ $$$$ /$$____/ |_____  $$
// | $$      | $$    $$| $$  | $$      | $$      | $$ \ $$$| $$       /$$  \ $$
// | $$      |  $$$$$$/|  $$$$$$/      | $$$$$$$$|  $$$$$$/| $$$$$$$$|  $$$$$$/
// |__/       \______/  \______/       |________/ \______/ |________/ \______/ 


#include "locomotivebehavior.h"
#include "ctrain_handler.h"
#include "sharedsection.h"

void LocomotiveBehavior::run()
{
    //Initialisation de la locomotive
    loco.allumerPhares();
    loco.demarrer();
    loco.afficherMessage("Ready!");

    /* A vous de jouer ! */

    // Vous pouvez appeler les méthodes de la section partagée comme ceci :
    //sharedSection->access(loco);
    //sharedSection->leave(loco);
    //sharedSection->stopAtStation(loco);

    while(true) {

        int pointAccess = 0; // c'est vraiment du bricolage, y a sûrement un meilleur moyen de faire
                            // en l'occurence ça marche par coincidence, grâce au circuit qu'on a choisi

        if (direction == SharedSectionInterface::Direction::D1) {
            for (std::pair<SharedSectionInterface::Direction, u_short> contact : contactPoints) {
                if (contact.first == direction && contact.second > pointAccess) {
                    pointAccess = contact.second;
                }
            }
        }
        else {
            pointAccess = 100;
            for (std::pair<SharedSectionInterface::Direction, u_short> contact : contactPoints) {
                if (contact.first == direction && contact.second < pointAccess) {
                    pointAccess = contact.second;
                }
            }
        }
        attendre_contact(pointAccess);
        sharedSection->access(loco, direction);
        //loco.afficherMessage("J'ai atteint le contact " + std::to_string(pointAccess));
        loco.afficherMessage("J'ai atteint le point d'entrée de la section partagée");
    }
}


void LocomotiveBehavior::printStartMessage()
{
    qDebug() << "[START] Thread de la loco" << loco.numero() << "lancé";
    loco.afficherMessage("Je suis lancée !");
}

void LocomotiveBehavior::printCompletionMessage()
{
    qDebug() << "[STOP] Thread de la loco" << loco.numero() << "a terminé correctement";
    loco.afficherMessage("J'ai terminé");
}
