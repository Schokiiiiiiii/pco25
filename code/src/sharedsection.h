//  /$$$$$$$   /$$$$$$   /$$$$$$         /$$$$$$   /$$$$$$   /$$$$$$  /$$$$$$$
// | $$__  $$ /$$__  $$ /$$__  $$       /$$__  $$ /$$$_  $$ /$$__  $$| $$____/
// | $$  \ $$| $$  \__/| $$  \ $$      |__/  \ $$| $$$$\ $$|__/  \ $$| $$
// | $$$$$$$/| $$      | $$  | $$        /$$$$$$/| $$ $$ $$  /$$$$$$/| $$$$$$$
// | $$____/ | $$      | $$  | $$       /$$____/ | $$\ $$$$ /$$____/ |_____  $$
// | $$      | $$    $$| $$  | $$      | $$      | $$ \ $$$| $$       /$$  \ $$
// | $$      |  $$$$$$/|  $$$$$$/      | $$$$$$$$|  $$$$$$/| $$$$$$$$|  $$$$$$/
// |__/       \______/  \______/       |________/ \______/ |________/ \______/


#ifndef SHAREDSECTION_H
#define SHAREDSECTION_H

#include <QDebug>
#include <unistd.h>

#include <pcosynchro/pcosemaphore.h>

#ifdef USE_FAKE_LOCO
#  include "fake_locomotive.h"
#else
#  include "locomotive.h"
#endif

#ifndef USE_FAKE_LOCO
  #include "ctrain_handler.h"
#endif

#include "sharedsectioninterface.h"

/**
 * @brief La classe SharedSection implémente l'interface SharedSectionInterface qui
 * propose les méthodes liées à la section partagée.
 */
class SharedSection final : public SharedSectionInterface
{
public:

    /**
     * @brief SharedSection Constructeur de la classe qui représente la section partagée.
     * Initialisez vos éventuels attributs ici, sémaphores etc.
     */
    SharedSection() = default; // modified - Fabien

    /**
     * @brief Request access to the shared section
     * @param Locomotive who asked access
     * @param Direction of the locomotive
     */
    void access(Locomotive& loco, Direction d) override {

        // modified - Fabien

        // acquire mutex
        mutex.acquire();

        // if loco already accessed, cancel
        if (this->loco == &loco || stopped) {
            ++countErrors;
            mutex.release();
            return;
        }

        // if it's already occupied
        if (occupied) {

            // wait and add a loco waiting
            loco.arreter();
            ++nbWaiting;
            mutex.release();

            // wait for a signal that loco has left
            left.acquire();

            // if there was a stopAll, cancel restart
            if (stopped)
                return;

            // for passengers' heart, sleep at least 1s
            sleep(1);

            // replace current loco in the shared section
            mutex.acquire();
            this->loco = &loco;
            this->direction = d;
            mutex.release();

            // start the loco again
            loco.demarrer();

        // if it's not occupied
        } else {

            // loco enters the shared section
            this->loco = &loco;
            this->direction = d;
            occupied = true;
            mutex.release();
        }
    }

    /**
     * @brief Notify the shared section that a Locomotive has left (not freed yed).
     * @param Locomotive who left
     * @param Direction of the locomotive
     */
    void leave(Locomotive& loco, Direction d) override {

        // modified - Fabien

        // change occupied to false, but don't release yet
        mutex.acquire();

        // if not occupied or wrong loco is leaving or wrong direction, cancel
        if (occupied == false || this->loco != &loco || this->direction != d) {
            ++countErrors;
            mutex.release();
            return;
        }

        // section is not occupied anymore
        occupied = false;

        // release mutex;
        mutex.release();
    }

    /**
     * @brief Notify the shared section that it can now be accessed again (freed).
     * @param Locomotive who sent the notification
     */
    void release(Locomotive &loco) override {

        // modified - Fabien

        // acquire mutex
        mutex.acquire();

        // if it didn't call for leave before or not the right loco, cancel
        if (occupied || this->loco != &loco) {
            ++countErrors;
            mutex.release();
            return;
        }

        // if there is a loco waiting release it and change to occupied
        if (nbWaiting > 0) {
            --nbWaiting;
            occupied = true;
            left.release();
        } else {
            this->loco = nullptr;
        }

        // release mutex
        mutex.release();
    }

    /**
     * @brief Stop all locomotives to access this shared section
     */
    void stopAll() override {

        // modified - Fabien

        // acquire mutex so nothing else happens
        mutex.acquire();

        // we stop the possibility to start locos again and release any waiting threads
        stopped = true;
        for (int i = 0 ; i < nbWaiting ; ++i)
            left.release();
        nbWaiting = 0;

        // release mutex
        mutex.release();
    }

    /**
     * @brief Return nbErrors
     * @return nbErrors
     */
    int nbErrors() override {

        // modified - Fabien

        // simply return the number of calls
        return countErrors;
    }

private:
    /*
     * Vous êtes libres d'ajouter des méthodes ou attributs
     * pour implémenter la section partagée.
     */
    PcoSemaphore mutex  = PcoSemaphore(1);  // protects occupied decision making and nbWaiting
    PcoSemaphore left   = PcoSemaphore(0);  // if section is occupied, makes other locomotives wait (barrier)
    bool occupied       = false;              // true if there is a locomotive in the shared section, false otherwise
    int nbWaiting       = 0;                  // number of locomotives waiting to access shared section
    int countErrors     = 0;                  // count the number of errors because of wrong calls to SharedSection
    Locomotive *loco    = nullptr;            // current loco in the section
    Direction direction = Direction::D1;      // direction of the loco inside the section (by default D1)
    bool stopped        = false;              // used to stop all locomotives
};


#endif // SHAREDSECTION_H
