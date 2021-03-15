## project_pandOS
In questi file è contenuta l'implementazione del livello 2 di pandOS. Attraverso le funzioni qui descritte possiamo inizializzare le code dei processi o dei semafori e gestirne gli elementi.

## Autori
  - Bassoli Lorenzo
  - Gorini Letizia 
  - Manzano Gerald
  - Pirazzoli Leon 

## Inizializzazione
Per prima cosa utilizziamo **InitPcbs()** e **initASL()** per inizializzare le strutture dati contenenti i processi (**"pcbFree_h"**) e i semafori liberi (**"semdFree_h"**).

## Gestione pcb_t
Possiamo allocare e liberare i processi con **allocPcb()**, deallocarli con **freePcb()** e gestirli con altre funzioni come  **insertProcQ()** e  **removeProcQ()**.
Infine abbiamo implementato delle funzioni che ne gestiscono le relazioni di parentela come **insertChild()**, **outChild()**, **removeChild()** e **emptyChild()**. 


## Gestione semd_t
La lista dei semafori attivi è ordinata in base al descrittore semAdd di ciascun semaforo in modo da rendere più efficace la ricerca e ridurre il costo computazionale della funzione.
Inoltre abbiamo un nodo iniziale con *semAdd uguale a 0 e un nodo finale con *semAdd pari a infinito, che ci semplifica la gestione della Active Semaphore List.
Per gestire i semafori tra le varie code utilizzando le funzioni di rimozione e inserimento removeBlocked(), outBlocked() e insertBlocked(). In particolare utilizziamo insertBlocked quando vogliamo aggiungere un processo in coda ad un determinato semaforo e quindi alla sua 's_procQ'. 
Nel caso il semaforo con il rispettivo semAdd non sia presente nella ASL, lo aggiungiamo prendendone uno dalla lista dei semafori liberi rispettando l'ordine.
La rimozione di un processo bloccato ad un semaforo viene attuata da removeBlocked e outBlocked, i quali rimuoveranno il semaforo dalla ASL nel caso la sua coda dei processi diventasse vuota, mantenendo sempre l'ordinamento.
