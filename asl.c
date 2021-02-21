#include "asl.h"
#define NULL 0

/*Viene inserito il PCB puntato da p nella coda dei processi bloccati associata al SEMD con chiave semAdd. Se il semaforo corrispondente non è presente nella ASL,
 * alloca un nuovo SEMD dalla lista di quelli liberi (semdFree) e lo inserisce nella ASL, settando I campi in maniera opportuna (i.e.
key e s_procQ). Se non è possibile allocare un nuovo SEMD perché la lista di quelli liberi è vuota, restituisce TRUE. In tutti gli altri casi, restituisce FALSE. */
int insertBlocked(int *semAdd, pcb_t *p){

    semd_PTR tmp = semd_h;

    if (semAdd != NULL && p != NULL){

        if(tmp == NULL){//se la lista dei semafori attivi è vuota
            printf ("case ASL vuota \n");
            printf ("alloco primo semd\n");

            //alloco il primo semaforo
            semd_PTR newsem = semdFree_h;
            printf("semdFree %d\n", semdFree_h);
            semdFree_h = semdFree_h->s_next;
            printf("semdFree %d\n", semdFree_h);

            newsem->s_next = NULL;
            newsem->s_procQ = mkEmptyProcQ();
            newsem->s_semAdd = semAdd;

            //aggiungo il semaforo nella lista dei semafori attivi ASL
            semd_h = newsem;
            p->p_semAdd = semAdd;
            printf ("inserisco p in semd con insertPQ...\n");
            //inserisco p nel nuovo semaforo
            insertProcQ( &( newsem->s_procQ ), p );
            return 0;

        }else if (semdFree_h == NULL) {//se la lista dei semafori attivi è piena (ovvero lista semafori liberi vuota)

                printf ("case ASL piena\n");
                return 1;

            }else{//se la lista sei semafori attivi contiene almeno un elemento ma non è piena
                printf ("case ASL non piena\n");

                //cerco il semaforo nella lista dei semafori allocati
                while(tmp != NULL){
                    if( tmp->s_semAdd == semAdd ){
                        p->p_semAdd = semAdd;
                        insertProcQ(&(tmp->s_procQ), p);
                        return 0;
                    }

                    tmp = tmp->s_next;
                }

                printf ("?NOT FOUND\n");
                //se non ho trovato il semaforo tra i semafori allocati allora aggiungo un semaforo
                //alloco un nuovo semaforo

                semd_PTR newsem = semdFree_h;
                printf("semdFree %d\n", semdFree_h);
                semdFree_h = semdFree_h->s_next;
                printf("semdFree %d\n", semdFree_h);

                newsem->s_procQ = mkEmptyProcQ();
                newsem->s_semAdd = semAdd;

                //aggiungo il semaforo nella lista dei semafori attivi ASL
                newsem->s_next = semd_h;
                semd_h = newsem;

                //inserisco p nel nuovo semaforo
                printf ("%d  %d \n", p->p_semAdd, semAdd);
                p->p_semAdd = semAdd;

                printf ("why???\n");
                insertProcQ(&(newsem->s_procQ), p);

                return 0;
            }

    }else return 0;
}


/*Ritorna il primo PCB dalla coda dei processi bloccati (s_procq) associata al SEMD della ASL con chiave semAdd. Se tale descrittore non esiste nella ASL,
 * restituisce NULL. Altrimenti, restituisce l’elemento rimosso. Se la coda dei processi bloccati per il semaforo diventa vuota, rimuove il descrittore
corrispondente dalla ASL e lo inserisce nella coda dei descrittori liberi (semdFree_h). */

//PROCQ PUNTA ALLA TESTA DELLA CODA E NON ALLA CODA DELLA CODA !!!!!!!!!!!!!!!!!!!!!!!!

pcb_t* removeBlocked(int *semAdd){

    if (semAdd != NULL){    //se la lista di semafori attivi è vuota, non ho niente da rimuovere
        if(semd_h == NULL){
            printf ("case semd_h == NULL \n");
            return NULL;

        }else{ //se la lista di semafori attivi ha almeno un elemento
            printf("case ASL ha almeno un elem\n");
            pcb_PTR elem_toremove = NULL;
            semd_PTR tmp = semd_h;

            semd_PTR old_tmp = NULL;
            int found = 0;

            while(tmp != NULL && found == 0){
                if(tmp->s_semAdd == semAdd){ //ho trovato il semaforo
                    found = 1;
                    if(tmp->s_procQ->p_next == tmp->s_procQ){ // caso in cui c'è un solo elem da rimuovere, sposto codesto semaforo tra quelli liberi
                            printf ("case solo un  elemento\n");
                            if(old_tmp == NULL){ //se il puntatore al semaforo precedente è null ovvero il semaforo trovato è il primo,  rimuoviamo il semaforo e spostiamo la sentinella
                                printf ("case primo semd\n");
                                semd_h = tmp->s_next;
                                tmp->s_procQ = NULL;
                                tmp->s_next = semdFree_h;
                                semdFree_h = tmp;

                            }else{
                                printf ("case non primo semd\n");
                                tmp->s_procQ = NULL;
                                old_tmp->s_next = tmp->s_next; //togliamo il semaforo  dalla lista di semafori bloccati (asl)
                                tmp->s_next = semdFree_h; //aggiungiamo il semaforo alla lista di semafori liberi
                                semdFree_h = tmp;
                            }
                    }
                    printf ("*****************\n");

                    elem_toremove = removeProcQ(&(tmp->s_procQ));

                    printf ("***********111******  %d\n", tmp->s_procQ);

                    //outChild(elem_toremove);
                    printf ("---------------------------\n");
                }
                old_tmp = tmp;
                tmp = tmp->s_next;
            }

            //se non lo trovo
            if (found == 0)
                return NULL;

            return elem_toremove;
      }
    }else return NULL;
}

/*
 * Rimuove il PCB puntato da p dalla coda delsemaforo su cui è bloccato (indicato da p->p_semAdd). Se il PCB non compare in tale coda, allora restituisce NULL (condizione di errore).
Altrimenti, restituisce p.  */
pcb_t* outBlocked(pcb_t *p){
    if (p != NULL){
        if(p->p_semAdd == NULL) //se il semaforo di p non esiste return NULL
            return NULL;

        else{ //se il semaforo di p esiste
            semd_PTR tmp = semd_h;
            semd_PTR old_tmp = NULL;
            pcb_PTR elem_toremove = NULL;
            int found = 0;

            while(tmp != NULL && found == 0){ //cerco il semaforo di p nella lista di semafori attivi (ASL)
                if(tmp->s_semAdd == p->p_semAdd)
                    found = 1;
                else{
                    old_tmp = tmp;
                    tmp = tmp->s_next;
                }
            }

            if(found == 1){ //se il semaforo è presente nella lista dei semafori attivi
                if(tmp->s_procQ->p_next == tmp->s_procQ){ //se c'è un solo processo nella coda del semaforo
                    if(old_tmp == NULL){ //se il puntatore al semaforo precedente è null ovvero il samaforo trovato è il primo,  rimuoviamo il semaforo e spostiamo la sentinella
                        semd_h = tmp->s_next;
                        tmp->s_next = semdFree_h;
                        semdFree_h = tmp;
                    }else{
                        old_tmp->s_next = tmp->s_next; //togliamo il semaforo  dalla lista di semafori bloccati (asl)
                        tmp->s_next = semdFree_h; //aggiungiamo il semaforo alla lista di semafori liberi
                        semdFree_h = tmp;
                    }
                }
                elem_toremove = outProcQ( &(tmp->s_procQ), p );
                outChild(elem_toremove);
            }
            else
                return NULL;

            return elem_toremove;
        }
    }else return NULL;
}

/*Restituisce (senza rimuovere) il puntatore al PCB che si trova in testa alla coda dei processiassociata al SEMD con chiave semAdd. Ritorna NULL se il SEMD non compare nella ASL oppure
se compare ma la sua coda dei processi è vuota.*/
pcb_t* headBlocked(int *semAdd){
    if (semAdd != NULL){
        semd_PTR tmp = semd_h;

        while(tmp != NULL ){
            if(tmp->s_semAdd == semAdd){
                /*if(tmp->s_procQ == NULL) {return ERRORE MOOOOOOOOOOOOLTO GRAVE (LETIZIA);}*/
                return headProcQ( &(tmp->s_procQ) );
            }
            tmp = tmp->s_next;
        }
        return NULL;
    }else return NULL;
}

/*Inizializza la lista dei semdFree in modo da contenere tutti gli elementi della semdTable. Questo metodo viene invocato una volta sola durante
l’inizializzazione della struttura dati*/
void initASL(){
    int i;
    for ( i=0 ; i<MAXPROC; i++ ){
        semd_table[i].s_next = NULL;
        semd_table[i].s_semAdd = NULL;
        semd_table[i].s_procQ = NULL;
    }

    //setto sentinella semafori liberi
    semdFree_h = & semd_table[0];

    //primo semaforo
    semd_table[0].s_next = & semd_table[1];
    for ( i=1; i<MAXPROC-1 ; i++ ) {
        semd_table[i].s_next = & semd_table[i+1];
    }
    //ultimo semaforo
    semd_table[MAXPROC-1].s_next = NULL;

}
