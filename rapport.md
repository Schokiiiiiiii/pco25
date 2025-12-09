# Lab05 - BikeStation

## Général

- Auteurs: Fabien Léger & Aymeric Bonny
- Cours: PCO, HEIG-VD
- Data: 09.12.2025

## Problématique

Il était demandé dans ce labo de créer un système de vélos libres-services.

Les personnes peuvent prendre et mettre des vélos à des stations. Si la station est vide, ils attendent qu'un vélo soit
déposé et vice-versa si la station est pleine.

Il faut ainsi mettre en place un moniteur de Mesa pour faire attendre les personnes avant qu'ils puissent prendre ou
déposer un vélo dépendant de l'état de la station.

## Implémentation

### Person

### Van

### BikeStation

Au niveau de l'implémentation, nous avons décidé de partir sur des queues de condition. Grâce à cela, on évite un
nombre limite qu'on aurait avec une machine à ticket (même si celle-ci est très grande).

Nous avons décomposé le problème en différents attributs

| Variable       | Type                          | Description                                                                                                                                                                                   |
|----------------|-------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| mutex          | PcoMutex                      | Protéger pour avoir uniquement une personne parcourant les `getBike()`/`putBike()`/`addBikes()`/`putBikes()` et ainsi protéger les différentes variables comme le demande un moniteur de Mesa |
| bikeGetPerType | Array de Queues de Conditions | Les personnes prennent les vélos selon le type (array du nombre de types) et FIFO (queue)                                                                                                     |
| bikePut        | Queue de Conditions           | Les personnes posent les vélos indépendamment du type selon FIFO (queue)                                                                                                                      |
| bikesPerType   | Array de Queues de conditions | Les vélos sont rangés suivant le type (array du nombre de types) et FIFO (queue)                                                                                                              |
| stopped        | bool                          | Devient true si stopSimulation est appelé pour renvoyer les threads entrant et libérer ceux coincés                                                                                           |

Il a été nécessaire d'avoir des queues pour chaque structure pour les raisons suivantes :
- bikeGetPerType : on prend un vélo suivant le premier arrivé à la station (en plus du type pour pas attendre pour rien)
- bikePut : on met un vélo suivant le premier arrivé à la station
- bikesPerType : Il est nécessaire de prendre les vélos avec `getBikes()` selon type et FIFO donc on doit avoir une queue

On a surtout suivi le code qui nous était donnée pour un moniteur de Mesa sinon. La seule différence est que nous
n'avons pas eu besoin de variables pour compter, car toutes les conditions sont déjà dans nos queues et il est facile de
savoir combien il y en a avec la fonction `size()`.

## Tests

Tester person.h et van.h s'avère difficile, car ces classes sont privées. Il semble possible de mettre les tests en tant
que friend mais nous nous sommes vite fait rendu compte de la complexité de tester ces classes. Il fallait créer des
BikeStation valables, différents vélos, essayer différentes valeurs d'entrées.

Tester BikeStations aurait été le bienvenu, mais nous n'avons malheureusement pas vraiment eu le temps et il a été dit
sur le Teams que ceux-ci n'étaient pas nécessaires. Nous avons donc préféré rendre le code propre plutôt que faire de
nombreux tests.

## Conclusion

Il n'y a pas vraiment eu de problèmes lors de ce labo. Le plus difficile a été de réfléchir aux bonnes structure pour
faire fonctionner le moniteur de Mesa.