# Laboratoire 02

- Auteurs: Fabien Léger, Aymeric Bonny
- Cours: PCO, HEIG-VD

---

## Questions

### Questions 1

> Mesurez le gain de temps produit par votre version multi-threadée en faisant varier le nombre 
> de threads. Que remarquez-vous ?

Nous pouvons remarquer que la durée de temps que prennent les tests à s'exécuter est
différente en fonction du nombre de threads alloués au programme. Il semble que lorsque
nous augmentons le nombre de threads, le programme s'exécute plus rapidement, ce qui
est logique d'après la théorie vue en cours.

### Question 2

> Le gain de vitesse est-il linéaire avec le nombre de threads ? (Indiquez le nombre de threads
> alloués à la VM, il peut être changé dans les settings, ainsi que le nombre de threads de votre
> machine). Il y a-t-il une différence pour les nombre premiers et non premiers ?

Selon les tests effectués, le gain de temps n'est pas linéaire. Dans le programme main_test
fourni, les résultats des tests se présentent globalement ainsi:

- le test multi-thread à un thread est plus lent à s'exécuter que le test single-thread.
- Le test à deux threads est environ 1s plus rapide que le précédent.
- le test à trois threads semble gagner plus dans les 500ms.
  - Aymeric a eu des résultats où le test à trois threads était plus lent que le deux
  threads. Cela pourrait s'expliquer, car il utilise la VM avec moins de cœurs et donc
  les changements de contextes coûtent plus cher.
- Le dernier test se fait avec douze threads, soit quatre fois plus que le test précédent.
Aymeric obtient uniquement 500ms de moins tandis que Fabien 1s.

On peut remarquer donc que le gain en temps semble diminuer de manière "logarithmique"
non seulement car il n'y a plus vraiment de différence entre 10 et 11 threads, mais aussi
car le nombre de threads sur notre ordinateur est limité. Le main_benchmark soutient aussi
ces hypothèses.

La configuration du système d'Aymeric est la VM avec deux cœurs logiques, tandis que
son hôte en possède douze.

La configuration du système de Fabien est en natif avec huit cœurs.

Il y a une grande différence entre les nombres premiers et les non-premiers, ce qui a du
sens puisque le thread s'arrête lorsqu'il a trouvé un diviseur.