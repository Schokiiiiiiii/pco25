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

int nextPoint(std::array<u_short, 4> contactPoints, SharedSection::Direction direction, int previousPoint) {
    int nextPoint = 0;

    if (previousPoint == 0) {
        switch(direction) {
        case SharedSection::Direction::D1:
            nextPoint = contactPoints.at(0);
            break;
        case SharedSection::Direction::D2:
            nextPoint = contactPoints.at(2);
            break;
        default:
            break;
        }
        return nextPoint;
    }

    switch(direction) {
        case SharedSection::Direction::D1:
            nextPoint = contactPoints.at(1);
            break;
        case SharedSection::Direction::D2:
            nextPoint = contactPoints.at(3);
            break;
        default:
            break;
    }
    return nextPoint;

    // je pourrais peut etre compacter ces deux morceaux de code avec des clever tricks mais ca le rendrait super illisible donc chai pas
}

int directionChange(std::array<u_short, 4> contactPoints) {
    auto gen = std::bind(std::uniform_int_distribution<>(0,1), std::default_random_engine()); // code de stackoverflow
    int change = gen();
    if (contactPoints.at(0) == 12) return change * 18; // ici c'est un peu comme tester le numéro de la loco, y a peut etre un meilleur moyen
    else return change * 13;
}

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

        int currentPoint = 0;

        if (int changeDirection = directionChange(contactPoints); changeDirection) {

            attendre_contact(changeDirection);
            loco.inverserSens();

            switch(direction) {
            case SharedSection::Direction::D1:
                direction = SharedSection::Direction::D2;
                break;
            case SharedSection::Direction::D2:
                direction = SharedSection::Direction::D1;
                break;
            default:
                break;
            }
        }

        currentPoint = nextPoint(contactPoints, direction, currentPoint);
        attendre_contact(currentPoint);

        loco.afficherMessage(QString::fromStdString("J'ai atteint le contact " + std::to_string(currentPoint)));

        sharedSection->access(loco, direction);
        currentPoint = nextPoint(contactPoints, direction, currentPoint);
        attendre_contact(currentPoint);

        // C'EST QUOI LA SECTION PARTAGÉE -> c'est aux points de contacts, donc elle est différent pour les deux trains, kinda

        loco.afficherMessage(QString::fromStdString("J'ai atteint le contact " + std::to_string(currentPoint)));

        sharedSection->leave(loco, direction);
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