//  /$$$$$$$   /$$$$$$   /$$$$$$         /$$$$$$   /$$$$$$   /$$$$$$  /$$$$$$$ 
// | $$__  $$ /$$__  $$ /$$__  $$       /$$__  $$ /$$$_  $$ /$$__  $$| $$____/ 
// | $$  \ $$| $$  \__/| $$  \ $$      |__/  \ $$| $$$$\ $$|__/  \ $$| $$      
// | $$$$$$$/| $$      | $$  | $$        /$$$$$$/| $$ $$ $$  /$$$$$$/| $$$$$$$ 
// | $$____/ | $$      | $$  | $$       /$$____/ | $$\ $$$$ /$$____/ |_____  $$
// | $$      | $$    $$| $$  | $$      | $$      | $$ \ $$$| $$       /$$  \ $$
// | $$      |  $$$$$$/|  $$$$$$/      | $$$$$$$$|  $$$$$$/| $$$$$$$$|  $$$$$$/
// |__/       \______/  \______/       |________/ \______/ |________/ \______/ 

#ifndef LOCOMOTIVEBEHAVIOR_H
#define LOCOMOTIVEBEHAVIOR_H

#include "locomotive.h"
#include "launchable.h"
#include "sharedsectioninterface.h"

/**
 * @brief La classe LocomotiveBehavior représente le comportement d'une locomotive
 */
class LocomotiveBehavior : public Launchable
{
public:
    /*!
     * \brief locomotiveBehavior Constructeur de la classe
     * \param loco la locomotive dont on représente le comportement
     */
    LocomotiveBehavior(Locomotive& loco, std::shared_ptr<SharedSectionInterface> sharedSection):
        loco(loco),
        sharedSection(sharedSection)
    {
        // Eventuel code supplémentaire du constructeur
    }


protected:
    /*!
     * \brief run Fonction lancée par le thread, représente le comportement de la locomotive
     */
    void run() override;

    /*!
     * \brief printStartMessage Message affiché lors du démarrage du thread
     */
    void printStartMessage() override;

    /*!
     * \brief printCompletionMessage Message affiché lorsque le thread a terminé
     */
    void printCompletionMessage() override;

    /**
     * @brief loco La locomotive dont on représente le comportement
     */
    Locomotive& loco;

    /**
     * @brief sharedSection Pointeur sur la section partagée
     */
    std::shared_ptr<SharedSectionInterface> sharedSection;

    // Le sens D1 est le sens dans lequel les locos commencent, càd le sens horaire
    std::array<std::pair<SharedSectionInterface::Direction, u_short>, 4> aContacts = {{SharedSectionInterface::Direction::D1, 12},
                                                                                      {SharedSectionInterface::Direction::D1, 4},
                                                                                      {SharedSectionInterface::Direction::D2, 5},
                                                                                      {SharedSectionInterface::Direction::D2, 11}};
    std::array<std::pair<SharedSectionInterface::Direction, u_short>, 4> bContacts = {{SharedSectionInterface::Direction::D1, 10},
                                                                                      {SharedSectionInterface::Direction::D1, 4},
                                                                                      {SharedSectionInterface::Direction::D2, 3},
                                                                                      {SharedSectionInterface::Direction::D2, 11}};

    /*
     * Vous êtes libres d'ajouter des méthodes ou attributs
     *
     * Par exemple la priorité ou le parcours
     */
};

#endif // LOCOMOTIVEBEHAVIOR_H
