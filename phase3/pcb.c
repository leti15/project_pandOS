#include "pcb.h"

static pcb_t pcbFree_table[MAXPROC];
static pcb_PTR pcbFree_h;
extern void bp();

/*Inizializza la pcbFree in modo da contenere tutti gli elementi della pcbFree_table.
 * Questo metodo deve essere chiamato una volta sola in fase di inizializzazione della struttura dati.*/
void initPcbs(){
    /* pcbFree_table tutti =NULL
        pcbFree_h sar� Null
    */
    for (int i=0; i<MAXPROC; i++){
        pcbFree_table[i].p_next = NULL;
        pcbFree_table[i].p_prev = NULL;
        pcbFree_table[i].p_prnt = NULL;
        pcbFree_table[i].p_child = NULL;
        pcbFree_table[i].p_next_sib = NULL;
        pcbFree_table[i].p_prev_sib = NULL;
        pcbFree_table[i].p_time = 0;
        pcbFree_table[i].p_semAdd = NULL;
        pcbFree_table[i].p_supportStruct = NULL;
    }

    //creo sentinella
    pcbFree_h = &pcbFree_table[0];

    //primo pcb
    pcbFree_table[0].p_next = &pcbFree_table[1];
    for (int i=1; i<MAXPROC-1; i++) {
        pcbFree_table[i].p_next= &pcbFree_table[i+1];
    }
    //ultimo pcb
    pcbFree_table[MAXPROC-1].p_next= NULL;
}

/* inserisce il PCB puntato da p nella lista dei PCB liberi (pcbFree_h)*/
void freePcb(pcb_t *p){

    if (pcbFree_h != NULL){ //se pcbFree_h non � vuota
        p->p_next = pcbFree_h;
        pcbFree_h = p;
    } else{ //se � vuota aggiungo p come unico elemento
        p->p_next = NULL;
        pcbFree_h = p;
    }

}

/*Restituisce NULL se la pcbFree_h � vuota. Altrimenti rimuove un elemento dalla pcbFree,
 * inizializza tutti i campi (NULL/0) e restituisce l�elemento rimosso.*/
pcb_t *allocPcb(){
    if (pcbFree_h == NULL)
        return NULL;
    else
    {
        //tmp � l'elemento estratto dalla coda
        pcb_PTR tmp = pcbFree_h;

        if (pcbFree_h->p_next != NULL){//se non � l'ultimo elemento
            //La sentinella punta l'elemento successivo a tmp
            pcbFree_h = pcbFree_h->p_next;
        }else { pcbFree_h = NULL; } //se � l'ultimo elemento metto la sentinella a null

        //reset di tmp
        tmp->p_child = NULL;
        tmp->p_next = NULL;
        tmp->p_prev = NULL;
        tmp->p_next_sib = NULL;
        tmp->p_prev_sib = NULL;
        tmp->p_prnt = NULL;
        tmp->p_semAdd = NULL;
        tmp->p_time = 0;
        tmp->p_supportStruct = NULL;

        return tmp;
    }
}

/* Crea una lista di PCB, inizializzandola come lista vuota (i.e. restituisce NULL)*/
pcb_t* mkEmptyProcQ() {
    return NULL;
}

/*Restituisce TRUE se la lista puntata da head � vuota, FALSE altrimenti*/
int emptyProcQ(pcb_t* tp) {
    if (tp == NULL)
        return 1;
    else
        return 0;
}

/*inserisce l�elemento puntato da p nella coda dei processi tp.
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

/*Restituisce l�elemento in fondo alla coda (ultimo immesso) dei processi tp, SENZA RIMUOVERLO.
Ritorna NULL se la coda non ha elementi.*/
pcb_t* headProcQ(pcb_t** tp){
    if (tp != NULL)
        return *tp; //restituiamo la sentinella che punta alla fine della coda
    else
        return NULL;
}

/*Rimuove l�elemento piu� vecchio dalla coda tp. Ritorna NULL se la coda � vuota, altrimenti ritorna il puntatore
all�elemento rimosso dalla lista.*/
pcb_t* removeProcQ(pcb_t **tp){

    if (tp != NULL)
    {
        if(*tp == NULL)
        {

            return NULL;
        }
        else
        {
            //primo elemento sentinella
            pcb_PTR sent_tp = *tp;
            pcb_PTR elem_toremove = (*tp)->p_next;
            if(sent_tp->p_next == sent_tp || sent_tp->p_prev == sent_tp)
            { //c'� un solo pcb

                (*tp) = NULL;
                return elem_toremove;
            }else
            {
                    elem_toremove=sent_tp->p_next;
                    //ultimo elemento = sent_tp
                    sent_tp->p_next = elem_toremove->p_next; // l'ultimo elemento punter� al secondo
                    elem_toremove->p_next->p_prev = sent_tp; //il prev del secondo punter� all'ultimo

                    return elem_toremove;
            }
        }
    }else return NULL;
}

/*Rimuove il PCB puntato da p dalla coda dei processi puntata da tp. Se p non � presente nella coda, restituisce NULL (p pu� trovarsi in una posizione arbitraria della coda).*/
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
                    //caso in cui l'elemento � in mezzo ad altri elementi
                    elem_toremove->p_prev->p_next = elem_toremove->p_next;
                    elem_toremove->p_next->p_prev = elem_toremove->p_prev;

                    //se l'elemento da rimuovere � la sentinella ovvero l'ultimo elemento
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
void insertChild(pcb_t *prnt, pcb_t *p)
{   
    if (prnt != NULL && p != NULL)
    {   bp();
        if(prnt->p_child != NULL)
        {   bp1();
            //troviamo il pcb esistente che punter� al nuovo pcb
            pcb_t * last = prnt->p_child->p_prev_sib;
            bp3();


            if(last == NULL)
                boo();

            //l'ultimo figlio punta al nuovo figlio
            last->p_next_sib = p;

            

            p->p_prev_sib = last;
            p->p_prnt = prnt;
            p->p_next_sib = prnt->p_child;
            prnt->p_child->p_prev_sib = p;
           
        }else{
            bp();
            prnt->p_child = p;
            p->p_prnt = prnt;
            p->p_prev_sib = p;
            p->p_next_sib = p;
        }
    }
bp3();
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

        }else if(p->p_child->p_prev_sib == p->p_child->p_next_sib){
            pcb_PTR elem_toremove = p->p_child;
            p->p_child = p->p_child->p_next_sib;
            p->p_child->p_next_sib = p->p_child;
            p->p_child->p_prev_sib = p->p_child;
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
altrimenti restituisce l�elemento rimosso (cio� p). A differenza della removeChild, p pu� trovarsi in una posizione arbitraria (ossia non �
necessariamente il primo figlio del padre).*/
pcb_t *outChild(pcb_t* p){
    if (p != NULL){
        if(p->p_prnt == NULL || p->p_prnt->p_child == p || p->p_next_sib == p){//se p non ha un padre oppure se p � l'unico figlio oppure � il primo figlio
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
