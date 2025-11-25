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

    if (previousPoint == 0) { //  premier point dans la loop
        switch(direction) {
        case SharedSection::Direction::D1:
            return contactPoints.at(0);
        case SharedSection::Direction::D2:
            return contactPoints.at(2);
        default:
            return previousPoint;
        }
    }

    // troisième point dans la loop (le train est arrivé à la fin de la section partagée)
    if (direction == SharedSection::Direction::D1 && previousPoint == contactPoints.at(2)) return contactPoints.at(3);
    else if (direction == SharedSection::Direction::D2 && previousPoint == contactPoints.at(0)) return contactPoints.at(1);

    switch(direction) { // deuxième  point dans la loop
        case SharedSection::Direction::D1:
            return contactPoints.at(2);
        case SharedSection::Direction::D2:
            return contactPoints.at(0);
        default:
            return previousPoint;
    }


    // je pourrais peut etre compacter ces deux morceaux de code avec des clever tricks mais ca le rendrait super illisible donc chai pas
}

int directionChange() {
    int change = 0;
    auto gen = std::bind(std::uniform_int_distribution<>(0, 1), std::default_random_engine());
    for (int i = 0; i < 10; ++i) change += gen(); // pas random sur la vm
    return change % 2;
}

void LocomotiveBehavior::run()
{
    //Initialisation de la locomotive
    loco.allumerPhares();
    loco.demarrer();
    loco.afficherMessage("Ready!");

    static int directionCompare;
    static PcoSemaphore mutex(1);
    static int clac = 0; // pour diriger l'aguillage

    while(true) {
        directionCompare = 3;
        int currentPoint = 0;

        mutex.acquire();    // grâce à cette magouille suprême, directionCompare != 0 équivaut à: les trains vont dans des directions différentes
        directionCompare == 3 ? directionCompare = static_cast<int>(direction) : directionCompare -= static_cast<int>(direction);
        mutex.release();

        currentPoint = nextPoint(contactPoints, direction, currentPoint);
        attendre_contact(currentPoint);

        loco.afficherMessage(QString::fromStdString("J'ai atteint le contact " + std::to_string(currentPoint) + ", l'entrée de ma section partagée"));

        sharedSection->access(loco, direction);

        currentPoint = nextPoint(contactPoints, direction, currentPoint);
        attendre_contact(currentPoint);

        loco.afficherMessage(QString::fromStdString("J'ai atteint le contact " + std::to_string(currentPoint) + ", la sortie de ma section partagée"));

        sharedSection->leave(loco, direction);

        ++clac; // cette variable est partagée, mais elle est déjà protégée par le sémaphore de la section partagée
        diriger_aiguillage(3, (DEVIE + clac % 2), 0);
        diriger_aiguillage(4, (TOUT_DROIT + clac % 2), 0);
        diriger_aiguillage(7, (TOUT_DROIT + clac % 2), 0);
        diriger_aiguillage(8, (DEVIE + clac % 2), 0);

        if (directionCompare) sharedSection->release(loco);
        else {
            currentPoint = nextPoint(contactPoints, direction, currentPoint);
            attendre_contact(currentPoint);
            sharedSection->release(loco);
        }

        if (directionChange()) {
            loco.afficherMessage("Je change de sens!");

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