#ifndef PCB_H_INCLUDED
#define PCB_H_INCLUDED

typedef struct pcb_t {
    /* process queue fields */
    struct pcb_t *p_next; /* ptr to next entry */
    struct pcb_t *p_prev; /* ptr to previous entry */

    /* process tree fields */
    struct pcb_t    *p_prnt, /* ptr to parent*/
    *p_child, /* ptr to 1st child*/
    *p_next_sib,/* ptr to next sibling */
    *p_prev_sib;/* ptr to prev. sibling */

    /* process status information */
    //state_t p_s; processor state */
    //cpu_t p_time; //cpu time used by proc */
    int *p_semAdd; /* ptr to semaphore on */
    /* which proc is blocked */
    /* support layer information */
    //support_t *p_supportStruct;

} pcb_t, *pcb_PTR;

pcb_t pcbFree_table[MAXPROC];
pcb_PTR pcbFree_h;

/*Inizializza la pcbFree in modo da contenere tutti gli elementi della pcbFree_table.
 * Questo metodo deve essere chiamato una volta sola in fase di inizializzazione della struttura dati.*/
void initPcbs(){
    /* pcbFree_table tutti =NULL
        pcbFree_h sarà Null
    */
    int i;
    for (i=0; i<MAXPROC; i++){
        pcbFree_table[i].p_next= NULL;
        pcbFree_table[i].p_prev= NULL;
        pcbFree_table[i].p_prnt= NULL;
        pcbFree_table[i].p_child= NULL;
        pcbFree_table[i].p_next_sib= NULL;
        pcbFree_table[i].p_prev_sib= NULL;
        //pcbFree_table[i].p_s=
        //pcbFree_table[i].p_time=
        pcbFree_table[i].p_semAdd= NULL;
    }

    //creo sentinella
    pcbFree_h= &pcbFree_table[0];


    for (i=0; i<MAXPROC ;i++) {
        if(i == 0){//primo pcb
            pcbFree_table[i].p_next= &pcbFree_table[i+ 1];
            pcbFree_table[i].p_prev= &pcbFree_table[MAXPROC- 1];

        }
        else if(i==MAXPROC-1){//ultimo pcb
            pcbFree_table[i].p_next=&pcbFree_table[0];
            pcbFree_table[i].p_prev=&pcbFree_table[i -1];
        }
        else {
                pcbFree_table[i].p_next=&pcbFree_table[i+ 1];
                pcbFree_table[i].p_prev=&pcbFree_table[i- 1];
        }
    }

}

/* inserisce il PCB puntato da p nella lista dei PCB liberi (pcbFree_h)*/
void freePcb(pcb_t *p){
    int n_pcb_free = 0;
    pcb_PTR tmp = pcbFree_h;

    while(tmp != NULL){
        n_pcb_free = n_pcb_free+ 1;
        tmp = tmp->p_next;
    }
    //controlliamo che sia possibile aggiungere un pcb
    if (n_pcb_free == MAXPROC){
        printf("ERROR: Non è possibile aggiungere un pcb!!!!!");
    }
    else if (n_pcb_free == 0){
        p->p_next = p;
        p->p_prev = p;
        pcbFree_h = p;
    }
    else{
        // inserimento in coda
        //il "prev" del primo elemento diventerà p
        //il "next" dell'ultimo elemento diventerà p
        pcb_PTR temp = pcbFree_h->p_prev; //temp è l'ultimo pcb
        pcbFree_h->p_prev = p; //il primo punterà al nuovo p
        p->p_prev = temp;//il NUOVO ultimo (=p) avrà come precedente il VECCHIO ultimo (=temp)
        temp->p_next = p; //il VECCHIO ultimo avrà come successivo il NUOVO ultimo
        p->p_next = pcbFree_h; // il NUOVO ultimo avrà come successore la sentinella quindi il primo pcb
    }
}

/*Restituisce NULL se la pcbFree_h è vuota. Altrimenti rimuove un elemento dalla pcbFree,
 * inizializza tutti i campi (NULL/0) e restituisce l’elemento rimosso.*/
pcb_t *allocPcb(){
    if (pcbFree_h == NULL)
        return NULL;
    else
    {
        //tmp è l'elemento estratto dalla coda
        pcb_PTR tmp = pcbFree_h;
        //La sentinella punta l'elemento successivo a tmp
        pcbFree_h = tmp->p_next;
        //Il nuovo primo punta all'ultimo
        pcbFree_h->p_prev = tmp->p_prev;
        //L'ultimo punta al nuovo primo
        tmp->p_prev->p_next = pcbFree_h;
        //reset di tmp
        tmp->p_child = NULL;
        tmp->p_next = NULL;
        tmp->p_prev = NULL;
        tmp->p_next_sib = NULL;
        tmp->p_prev_sib = NULL;
        tmp->p_prnt = NULL;
        tmp->p_semAdd = NULL;

        return tmp;
    }
}

/* Crea una lista di PCB, inizializzandola come lista vuota (i.e. restituisce NULL)*/
pcb_t* mkEmptyProcQ() {
    /*pcb_PTR tmp;
    tmp= NULL;
    return tmp;*/ return NULL;
}

/*Restituisce TRUE se la lista puntata da head è vuota, FALSE altrimenti*/
int emptyProcQ(pcb_t* tp) {
    if (tp == NULL)
        return 1;
    else
        return 0;
}

/*inserisce l’elemento puntato da p nella coda dei processi tp.
 * La doppia indirezione su tp serve per poter inserire p come ultimo elemento della coda.*/
void insertProcQ(pcb_t** tp, pcb_t* p){
    if (tp != NULL && p != NULL){
        pcb_PTR sent = *tp;

        if (sent != NULL){//coda NON vuota
            sent->p_next->p_prev = p;
            p->p_next = sent->p_next;
            p->p_prev = sent;
            sent->p_next = p;

        }else{//coda vuota
            p->p_next = p;
            p->p_prev = p;
        }
        *tp = p;
    }
}

/*Restituisce l’elemento in fondo alla coda (ultimo immesso) dei processi tp, SENZA RIMUOVERLO.
Ritorna NULL se la coda non ha elementi.*/
pcb_t* headProcQ(pcb_t** tp){
    if (tp != NULL)
        return *tp; //restituiamo la sentinella che punta alla fine della coda
    else
        return NULL;
}

/*Rimuove l’elemento piu’ vecchio dalla coda tp. Ritorna NULL se la coda è vuota, altrimenti ritorna il puntatore all’elemento
rimosso dalla lista.*/
pcb_t* removeProcQ(pcb_t **tp){

    if (tp != NULL){
        if(*tp == NULL)
            return NULL;
        else {
            //primo elemento sentinella
            pcb_PTR sent_tp = *tp;
            pcb_PTR elem_toremove = (*tp)->p_next;
            if(sent_tp->p_next == sent_tp){ //c'è un solo pcb

                *tp=NULL;
                return elem_toremove;
            }else{
                    elem_toremove=sent_tp->p_next;
                    //ultimo elemento = sent_tp
                    sent_tp->p_next = elem_toremove->p_next; // l'ultimo elemento punterà al secondo
                    elem_toremove->p_next->p_prev = sent_tp; //il prev del secondo punterà all'ultimo

                    return elem_toremove;
                }
            }
    }else return NULL;
}

/*Rimuove il PCB puntato da p dalla coda dei processi puntata da tp. Se p non è presente nella coda, restituisce NULL (p può trovarsi in una posizione arbitraria della coda).*/
pcb_t* outProcQ(pcb_t **tp, pcb_t *p) {
    if(tp != NULL && p != NULL){

        //primo elemento sentinella
        pcb_PTR sent_tp = *tp;

        //puntatore all'elemento da rimuovere
        pcb_PTR elem_toremove = NULL;

        if (*tp == NULL) {
            return NULL;

        }else{
            int found = 0;
            pcb_PTR tmp = sent_tp;

            do{
                    if(tmp == p){
                        found = 1;
                        elem_toremove = tmp;
                    }
                    tmp = tmp->p_next;

            }while(found == 0 && tmp != sent_tp);

            if(found == 1 )// ho trovato il pcb da rimuovere
            {
                //caso in cui elemento da rimuovere sia uno solo
                if (elem_toremove->p_prev == elem_toremove) {
                    *tp = NULL;

                }else {
                    //caso in cui l'elemento è in mezzo ad altri elementi
                    elem_toremove->p_prev->p_next = elem_toremove->p_next;
                    elem_toremove->p_next->p_prev = elem_toremove->p_prev;

                    //se l'elemento da rimuovere è la sentinella ovvero l'ultimo elemento
                    if (elem_toremove == sent_tp)
                        *tp = elem_toremove->p_prev;
                }
            }
        return elem_toremove;
        }
    } else return NULL;
}

/*Restituisce TRUE se il PCB puntato da p non ha figli, FALSE altrimenti. */
int emptyChild(pcb_t *p){
    if (p != NULL)
        if(p->p_child == NULL)
            return 1;

    return 0;
}

/*Inserisce il PCB puntato da p come figlio del PCB puntato da prnt.*/
void insertChild(pcb_t *prnt, pcb_t *p){
if (prnt != NULL && p != NULL){
        if(prnt->p_child != NULL){

            //troviamo il ptb esistente che punterà al nuovo ptb
            pcb_PTR last = prnt->p_child->p_prev_sib;

            //l'ultimo figlio punta al nuovo figlio
            last->p_next_sib = p;
            p->p_prev_sib = last;
            p->p_prnt = prnt;
            p->p_next_sib = prnt->p_child;
            prnt->p_child->p_prev_sib = p;

        }else{
            prnt->p_child = p;
            p->p_prnt = prnt;

            p->p_prev_sib = p;
            p->p_next_sib = p;
        }
    }else return NULL;
}

/*Rimuove il primo figlio del PCB puntato da p. Se p non ha figli, restituisce NULL.*/
pcb_t* removeChild(pcb_t *p){
    if ( p != NULL ){
        if(p->p_child == NULL){
            return NULL;
        }
        else if(p->p_child->p_prev_sib == p->p_child){
            pcb_PTR elem_toremove = p->p_child;
            p->p_child = NULL;

            //pulisco il puntatore all'elemento rimosso
            elem_toremove->p_next_sib = NULL;
            elem_toremove->p_prev_sib = NULL;
            elem_toremove->p_prnt = NULL;
            return elem_toremove;

        }else{
            pcb_PTR elem_toremove = p->p_child;
            pcb_PTR last = p->p_child->p_prev_sib;

            //collego il secondo elemento con l'ultimo
            last->p_next_sib = p->p_child->p_next_sib;
            p->p_child->p_next_sib->p_prev_sib = last;

            //collego il padre con il nuovo primo figlio
            p->p_child = p->p_child->p_next_sib;

            //pulisco il puntatore all'elemento rimosso
            elem_toremove->p_next_sib = NULL;
            elem_toremove->p_prev_sib = NULL;
            elem_toremove->p_prnt = NULL;
            return elem_toremove;
        }
    }else return NULL;
}

/*Rimuove il PCB puntato da p dalla lista dei figli del padre. Se il PCB puntato da p non ha un padre, restituisce NULL,
altrimenti restituisce l’elemento rimosso (cioè p). A differenza della removeChild, p può trovarsi in una posizione arbitraria (ossia non è
necessariamente il primo figlio del padre).*/
pcb_t *outChild(pcb_t* p){

    if (p != NULL){
        if(p->p_prnt == NULL || p->p_next_sib == p || p->p_prnt->p_child == p){//se p non ha un padre oppure se p è l'unico figlio oppure è il primo figlio
            return removeChild(p->p_prnt);

        }else{
            p->p_prev_sib->p_next_sib = p->p_next_sib;
            p->p_next_sib->p_prev_sib = p->p_prev_sib;

            //pulisco il puntatore all'elemento rimosso
            p->p_next_sib = NULL;
            p->p_prev_sib = NULL;
            p->p_prnt = NULL;
            return p;
        }
    }else return NULL;
}



#endif // PCB_H_INCLUDED
