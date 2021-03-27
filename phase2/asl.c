#include "asl.h"

#define NULL 0
#define MAXINT 0xFFFFFFFF

static semd_t semd_table[MAXPROC+2];
static semd_PTR semdFree_h;
static semd_PTR semd_h;

semd_PTR allocSemd() {
    if (semdFree_h == NULL){return NULL;} // se non ci sono più semafori allocabili return NULL

    int allocated = 0;
    semd_PTR newsemd = semdFree_h;
    semdFree_h = semdFree_h->s_next;

    //inizializzo campi newsemd
    newsemd->s_semAdd = (int*) 1; //setto una sola risorsa 
    newsemd->s_procQ = mkEmptyProcQ();

    //trovo dove metterlo nella ASL, dove mettere un semd da 1 risorsa
    semd_PTR temp = semd_h;
    while (temp->s_semAdd != (int*)MAXINT && allocated == 0){
        if (temp->s_next->s_semAdd > 1 || temp->s_next->s_semAdd == (*int) MAXINT){
            newsemd->s_next = temp->s_next;
            temp->s_next = newsemd;
            allocated = 1;
        }
        temp = temp->s_next;
    }
    //esce quando il semaforo è stato allocato 

    return newsemd;
}

/*Viene inserito il PCB puntato da p nella coda dei processi bloccati associata al SEMD con chiave semAdd. Se il semaforo corrispondente non è presente nella ASL,
 * alloca un nuovo SEMD dalla lista di quelli liberi (semdFree) e lo inserisce nella ASL, settando I campi in maniera opportuna (i.e.
key e s_procQ). Se non è possibile allocare un nuovo SEMD perché la lista di quelli liberi è vuota, restituisce TRUE. In tutti gli altri casi, restituisce FALSE. */
int insertBlocked(int *semAdd, pcb_t *p){

    semd_PTR tmp = semd_h;

    if (semAdd != NULL && p != NULL)
    {

        if (semdFree_h == NULL)
        {//se la lista dei semafori attivi è piena (ovvero lista semafori liberi vuota)
            return 1;
        }else
        {//se la lista sei semafori attivi contiene almeno un elemento ma non è piena

            //cerco il semaforo nella lista dei semafori allocati
            while(tmp != NULL)
            {
                if( tmp->s_semAdd == semAdd )
                {
                    p->p_semAdd = semAdd;   //se ho trovato il semaforo, lo aggiungo alla sua coda
                    insertProcQ(&(tmp->s_procQ), p);
                    return 0;
                } else if (tmp->s_next->s_semAdd > semAdd || tmp->s_next->s_semAdd == (int*)MAXINT) //se il semaforo dopo e' maggiore oppure se e' l'ultimo
                {
                    //se non ho trovato il semaforo tra i semafori allocati allora aggiungo un semaforo
                    //alloco un nuovo semaforo

                    semd_PTR sem_to_add = semdFree_h;
                    semdFree_h = semdFree_h->s_next;

                    sem_to_add->s_procQ = mkEmptyProcQ();
                    sem_to_add->s_semAdd = semAdd;

                    sem_to_add->s_next = tmp->s_next;
                    tmp->s_next = sem_to_add;

                    //inserisco p nel nuovo semaforo
                    p->p_semAdd = semAdd;

                    insertProcQ(&(sem_to_add->s_procQ), p);

                    return 0;
                }
                tmp = tmp->s_next;
            }
        }

    }else return 0;
}

/*Ritorna il primo PCB dalla coda dei processi bloccati (s_procq) associata al SEMD della ASL con chiave semAdd. Se tale descrittore non esiste nella ASL,
 * restituisce NULL. Altrimenti, restituisce l’elemento rimosso. Se la coda dei processi bloccati per il semaforo diventa vuota, rimuove il descrittore
corrispondente dalla ASL e lo inserisce nella coda dei descrittori liberi (semdFree_h). */

pcb_t* removeBlocked(int *semAdd)
{

    if (semAdd != NULL)
    {
        pcb_PTR elem_toremove = NULL;
        semd_PTR tmp = semd_h->s_next;

        semd_PTR old_tmp = semd_h;

        while(tmp->s_semAdd != (int*)MAXINT)
        {
            if(tmp->s_semAdd == semAdd)
            { //ho trovato il semaforo

                elem_toremove = removeProcQ(&(tmp->s_procQ));

                //controllo se dopo la rimozione il semaforo e' vuoto
                if (emptyProcQ(tmp->s_procQ))
                {
                    //lo rimuovo dalla ASL e dall' array dei device
                    remove_from_arrayDev(tmp->s_semAdd);
                    old_tmp->s_next = tmp->s_next;
                    tmp->s_next = semdFree_h;
                    semdFree_h = tmp;
                }

                return elem_toremove;
            }

            old_tmp = tmp;
            tmp = tmp->s_next;
        }

        return NULL;

    }
    else return NULL;
}

/*
 * Rimuove il PCB puntato da p dalla coda delsemaforo su cui è bloccato (indicato da p->p_semAdd). Se il PCB non compare in tale coda, allora restituisce NULL (condizione di errore).
Altrimenti, restituisce p.  */
pcb_t* outBlocked(pcb_t *p){
    if (p != NULL)
    {
        if(p->p_semAdd == NULL) return NULL;//se il semaforo di p non esiste return NULL
        else
        { //se il semaforo di p esiste
                semd_PTR tmp = semd_h->s_next;
                semd_PTR old_tmp = semd_h;
                pcb_PTR elem_toremove = NULL;

            while(tmp != NULL)
            { //cerco il semaforo di p nella lista di semafori attivi (ASL)
                if(tmp->s_semAdd == p->p_semAdd)
                {
                    elem_toremove = outProcQ( &(tmp->s_procQ), p );

                    if(emptyProcQ(tmp->s_procQ))
                    {   //se c'è un solo processo nella coda del semaforo lo rimuovo dalla ASL
                        old_tmp->s_next = tmp->s_next;
                        tmp->s_next = semdFree_h;
                        semdFree_h = tmp;
                        
                        //e lo rimuovo dall'array dei device
                        remove_from_arrayDev(tmp->s_semAdd);
                    }
                    return elem_toremove;
                }

                old_tmp = tmp;
                tmp = tmp->s_next;
            }

            return NULL;
        }

    } else return NULL;
}

/*Restituisce (senza rimuovere) il puntatore al PCB che si trova in testa alla coda dei processiassociata al SEMD con chiave semAdd. Ritorna NULL se il SEMD non compare nella ASL oppure
se compare ma la sua coda dei processi è vuota.*/
pcb_t* headBlocked(int *semAdd)
{
    if (semAdd != NULL)
    {
        semd_PTR tmp = semd_h;

        while(tmp != NULL )
        {
            if(tmp->s_semAdd == semAdd)
            {
                return (tmp->s_procQ->p_next);
            }
            tmp = tmp->s_next;
        }
        return NULL;
    }else return NULL;
}

/*Inizializza la lista dei semdFree in modo da contenere tutti gli elementi della semdTable. Questo metodo viene invocato una volta sola durante
l’inizializzazione della struttura dati*/
void initASL()
{
    int i;
    for ( i=0 ; i<MAXPROC+2; i++ ){
        semd_table[i].s_next = NULL;
        semd_table[i].s_semAdd = NULL;
        semd_table[i].s_procQ = mkEmptyProcQ();
    }

    //setto sentinella semafori liberi
    semdFree_h = & semd_table[1];

    semd_table[1].s_next = & semd_table[2]; //primo semaforo

    for ( i=2; i<MAXPROC+1 ; i++ ) {
        semd_table[i].s_next = & semd_table[i+1];
    }
    
    semd_table[MAXPROC].s_next = NULL; //ultimo semaforo
    semdFree_h = &semd_table[1]; //lista semafori liberi

    // Nodo ad inizio lista ASL
    semd_h = &semd_table[0];
    semd_h->s_semAdd = (int*)0;
    semd_h->s_procQ = NULL;

    // Nodo a fine lista ASL
    semd_h->s_next = &semd_table[MAXPROC +1];
    semd_h->s_next->s_semAdd = ( int*)MAXINT;
    semd_h->s_next->s_procQ = NULL;

    semd_h->s_next->s_next = NULL;
}
