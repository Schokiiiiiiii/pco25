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

        mutex.acquire();
        if (occupied) {
            loco.arreter();
            ++nbWaiting;
            mutex.release();
            left.acquire();
        } else {
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

        // if not occupied, no reason to leave
        if (occupied == false) {
            ++countErrors;
            return;
        }

        // change occupied to false, but don't release yet
        mutex.acquire();
        occupied = false;
        mutex.release();
    }

    /**
     * @brief Notify the shared section that it can now be accessed again (freed).
     * @param Locomotive who sent the notification
     */
    void release(Locomotive &loco) override {

        // modified - Fabien

        mutex.acquire();
        --nbWaiting;
        left.release();
        mutex.release();
    }

    /**
     * @brief Stop all locomotives to access this shared section
     */
    void stopAll() override {

        // modified - Fabien

        // we release any locomotive waiting so they can stop
        mutex.acquire();
        for (int i = 0 ; i < nbWaiting ; ++i)
            left.release();
        // maybe if a locomotive enters shared section
        // and there is also one inside it
        // it will block at mutex and then be released and never actually stop
        mutex.release();

        // TODO maybe do sth else ?
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
};


#endif // SHAREDSECTION_H
