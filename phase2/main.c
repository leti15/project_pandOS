#include <stdio.h>
#include <stdlib.h>

#include "pcb.h"
#include "asl.h"
#include "pandos_const.h"
#include "pandos_types.h"


/**
*   Variabili globali per:
*    - Conteggio dei processi vivi
*    - Conteggio dei processi bloccati
*    - Coda dei processi “ready”
*    - Puntatore al processo correntemente attivo
*    - Un semaforo (e.g. una variabile int) per ogni (sub) dispositivo. Non necessariamente tutti questi semafori sono sempre attivi.
*    - Strutture dati gia’ gestite in fase 1
*/

/*  da qualche parte va messo questo ma non so dove
    void uTLB_RefillHandler () {
        setENTRYHI(0x80000000);
        setENTRYLO(0x00000000);
        TLBWR();
        LDST ((state_PTR) 0x0FFFF000);
    }
*/
int proc_count;
int softB_count;
pcb_PTR readyQ;
pcb_PTR current_proc;
/** puntatore alla coda dei semafori attivi 'semd_h' */

int main()
{

/**  Da fare:
*    - Inizializzare i moduli di fase 1 ( initPcbs() e initSemd() )
*    - Inizializzare le variabili di cui sopra
*    - Popolare il pass up vector con gestore e stack pointer per eccezioni e TLB-Refill
*       !!capire il valore di STATE_GPR_LEN
*/

    initPcbs();
    initASL();




    printf("Hello world!\n");
    return 0;
}
