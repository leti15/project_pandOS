## Progetto PandOS 2020/21 - Fase 2

## Autori:

- Letizia Gorini
- Gerald Manzano
- Leon Pirazzoli

## Introduzione

Questa fase del progetto consiste nell'implementare le strutture e funzioni del livello 3 del nostro sistema operativo, rispettivamente:

- il gestore delle eccezioni: per occuparsi delle SYSCALL da 1 a 8, degli interrupt dei timer o device e di passare eccezioni di tipo TLB-refill o SYSCALL da 9 in su ai livelli superiori.

- lo scheduler con algoritmo di scheduling round-robin preemptive e time slice di 5 ms.

## SYSCALL

le SYSCALL gestite in questo livello vengono richieste da processi in kernel mode che salvano valori appropriati nei registri a0, a1, a2, a3 prima di eseguire la chiamata. 
A seconda dei valori contenuti nei registri possiamo dover fornire 8 servizi diversi (come creare un nuovo processo, terminare quello corrente, effettuare P-operation e V-operation etc...).
Al termine di una SYSCALL il controllo puo' essere restituito al processo corrente oppure puo' essere chiamato lo scheduler.

## Interrupt

Gli interrupt gestiti in questo livello possono derivare da:

- dispositivi I/O: 7 linee corrispondenti alle 7 classi di dispositivi (con eccezione dei dispositivi terminali che hanno 2 sub-device).

- timer: distinguibili tra Interval Timer (che lancia un interrupt ogni 100 ms) e Processor Local Timer (che lancia un interrupt ogni 5 ms).

## Non-timer interrupt

Quando il controller di un device causa un interrupt, significa che ha completato il task ad esso assegnato e attraverso l'interrupt handler gli consegnamo l'ACK dopo il quale torna disponibile ad accogliere la prossima richiesta (stato READY).
Le richieste vengono effettuate attraverso la SYSCALL5 (WaitforIO) che blocca il processo corrente al semaforo relativo al device effettuando una P-operation.
Abbiamo scelto di tracciare i semafori associati ai device utilizzando un array di interi (devicesem[DEVARRAYSIZE]) che rappresentano il numero di risorse (il semadd dei semafori). 
Il device numero x con numero di linea y corrisponde al semaforo:
 ** devicesem[(y-3)*8+x] **
\nInfine il 49esimo semaforo e' riservato all'Interval Timer.
La scelta di utilizzare un array di interi e' dovuta al fatto di avere locazioni di memoria contigue che ci permettono di capire se un semaforo e' associato a un device controllando se il suo indirizzo e' nel range [&devicesem[0], &devicesem[DEVARRAYSIZE]].
In questo modo possiamo tenere traccia dei processi soft-blocked, sia quelli che attendono che scatti l'Interval Timer, sia quelli che attendono una risposta da un device, in modo efficiente.

## Scheduler

Lo scheduler si occupa di gestire l'avvicendamento dei processi e di garantire il loro equale avanzamento ovvero la fairness. Cio' avviene caricando un nuovo processo dalla readyQueue ogni volta che scatta il PLT (anche nel caso in cuo il processo corrente non abbia completato il suo CPU burst), oppure in seguito a una SYSCALL bloccante (Wait_for_IO, Passeren, Wait_for_clock). Esso carichera' la time slice di 5 ms nel PLT e passera' il controllo al processo.
Nel caso in cui non ci fossero piu' processi che attendono di poter utilizzare la CPU, in base ai parametri che tengono traccia dei processi attivi e dei processi soft-blocked, esso decidera' se spegnere la macchina (HALT()), se metterla in attesa di un interrupt (WAIT()) oppure se andare in stato di errore (PANIC()).

## Come compilare e avviare

Compilare il makefile con il comando "make" da terminale quando si e' nella cartella contenente i file sorgente.

Su umps3 creare una nuova macchina, selezionando la cartella di cui sopra, attivare la virtual memory (0x8000.0000), accenderla e aprire il terminale (alt+0) e far partire l'esecuzione.
