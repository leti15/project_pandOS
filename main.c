#include <stdio.h>
#include <stdlib.h>
#include "struct_library.h"
#define MAXPROC 20

pcb_t pcbFree_table[MAXPROC];
pcb_PTR pcbFree_h;

semd_t semd_table[MAXPROC];
semd_PTR semdFree_h;
semd_PTR semd_h;


void initPcbs();
void freePcb(pcb_t *p);
pcb_t *allocPcb();
pcb_t* mkEmptyProcQ();
int emptyProcQ (pcb_t *tp);
void insertProcQ(pcb_t** tp, pcb_t* p);
pcb_t* headProcQ(pcb_t** tp);
pcb_t* removeProcQ(pcb_t **tp);
pcb_t* outProcQ(pcb_t **tp, pcb_t *p);

//tree
int emptyChild(pcb_t *p);
void insertChild(pcb_t*prnt, pcb_t *p);
pcb_t* removeChild(pcb_t *p);
pcb_t *outChild(pcb_t* p);

//gestione della ASL
int insertBlocked(int *semAdd,pcb_t *p);
pcb_t* removeBlocked(int *semAdd);
pcb_t* outBlocked(pcb_t *p);
pcb_t* headBlocked(int *semAdd);
void initASL();



//main
int main(){

    initPcbs();
    initASL();
/*
    for (int i=0; i<MAXPROC;i++){
        printf("\nfreeTab (%d): %d", i, &pcbFree_table[i]);

    }

    pcb_t prova;
    printf("\n\n ind prova: %d\n",&prova);
    freePcb(&prova);
    printf("\n\n prova alloc2: %d\n", allocPcb());
    pcb_PTR temp=pcbFree_h;
    for(int i=0; i<=n_pcb_free; i++){

        printf("\n\npcb(%d): %d", i, temp);
        printf("\tind_next:%d \ncont_next: %d", (temp->p_next), *(temp->p_next) );
        printf("\tind_prev:%d \ncont_prev: %d", (temp->p_prev), *(temp->p_prev) );
        temp=temp->p_next;
    }

    pcb_PTR prova=mkEmptyProcQ();
    /*printf("%d \n indirizzo contenuto: ",&prova);
    printf("%d \n contenuto:",prova);
    int a=emptyProcQ(prova);
    printf("%d \n",a);

    pcb_t boh;
    pcb_PTR p=&boh;
    pcb_PTR *tp=&prova;
    insertProcQ(tp,p);
    pcb_t boh2;
    pcb_PTR p2=&boh2;

    insertProcQ(tp,p2);
    pcb_t boh3;
    pcb_PTR p3=&boh3;
   // insertProcQ(tp,p3);
    /*printf("%d  boh \n  ",boh);
    printf("%d prev \n", (*tp)->p_prev);
    printf("%d  tp \n ", *tp);
    printf("%d  prova \n ", prova);
    printf("%d  indirizzo prova \n ", &prova);

    printf(" boh %d \n",boh);
    printf(" boh2 %d \n",boh2);
    //printf(" boh3 %d \n",boh3);
    printf("cont tp prima %d ",*tp);/*
    pcb_PTR h=headProcQ(tp);
    printf(" h %d \n",h);*/
/*pcb_PTR deleted=removeProcQ(tp);
    if(deleted == NULL){printf("NULL");}
    printf("removed %d \n",deleted);
    printf("cont tp %d ",*tp);
    pcb_PTR s=outProcQ(tp,p3);
    printf("contenuto s %d \n",s);
    printf("nuove sentinella %d \n",**tp);


    pcb_PTR hoppadre= &pcbFree_table[0];
    printf("hop:%d \n",emptyChild(hoppadre));
    pcb_PTR hop= &pcbFree_table[1];
    insertChild(hoppadre,hop);
    printf("hop:%d \n",emptyChild(hoppadre));
    pcb_PTR hop2= &pcbFree_table[2];
    insertChild(hoppadre,hop2);
    pcb_PTR hop3= &pcbFree_table[3];
    insertChild(hoppadre,hop3);

    printf("contenuto di hop %d \n",*hop);
    printf("contenuto di hop2 %d \n",*hop2);
    printf("contenuto di hop3 %d \n",*hop3);


    printf("contenuto del primo figlio: %d \n", *(hoppadre->p_child));
    printf("contenuto del secondo figlio: %d \n", *(hoppadre->p_child->p_next_sib));
    printf("contenuto del terzo figlio: %d \n", *(hoppadre->p_child->p_next_sib->p_next_sib));
    printf("contenuto di nuovo del primo figlio %d \n",*(hoppadre->p_child->p_next_sib->p_next_sib->p_next_sib) );

    printf("elem rimosso: %d \n",*outChild(hop3));

    printf("contenuto del primo figlio: %d \n", *(hoppadre->p_child));
    printf("contenuto del secondo figlio: %d \n", *(hoppadre->p_child->p_next_sib));
    //printf("contenuto del terzo figlio: %d \n", *(hoppadre->p_child->p_next_sib->p_next_sib));
    printf("contenuto di nuovo del primo figlio %d \n",*(hoppadre->p_child->p_next_sib->p_next_sib) );


    printf("elem rimosso: %d \n",*outChild(hop2));
    printf("elem rimosso: %d \n",*outChild(hop));
    printf("contenuto del primo figlio: %d \n", (hoppadre->p_child));

    /*
    printf("elem rimosso: %d \n",*removeChild(hoppadre));
    printf("elem rimosso: %d \n",*removeChild(hoppadre));
    printf("elem rimosso: %d \n",*removeChild(hoppadre));
    printf("elem rimosso: %d \n",removeChild(hoppadre));
    printf("hoppadre p child %d\n",hoppadre->p_child);
    */
    pcb_PTR asdrubale= allocPcb();
    pcb_PTR leon= allocPcb();
    pcb_PTR poveroger= allocPcb();

    printf("semh: %d\n" ,semd_h);
   int semadd=349;
   int control=insertBlocked(&semadd,asdrubale);
    printf("-----------\n");
    printf("semdh semadd 349: %d\n",*(semd_h->s_semAdd));
    printf("indirizzo di asdrubale: %d\n",asdrubale);
    printf("semdh sprocQ: %d\n",semd_h->s_procQ);

    control=insertBlocked(&semadd,leon);
    printf("-----------\n");
    printf("semdh semadd 349: %d\n",*(semd_h->s_semAdd));
    printf("indirizzo di letizia: %d\n",leon);
    printf("semdh sprocQ: %d\n",semd_h->s_procQ->p_next);


    int semadd2=22;
    control=insertBlocked(&semadd2,poveroger);
    printf("-----------\n");
    printf("semdh s next s semadd 22: %d\n",*(semd_h->s_semAdd));
    printf("indirizzo di poveroger: %d\n",poveroger);
    printf("semdh snext sprocQ: %d\n",semd_h->s_procQ);

    printf("-----------\n");
    printf("semdh sprocQ poverogerald: %d\n",semd_h->s_procQ);
    printf("semdh snext sprocQ asdrubale: %d\n",semd_h->s_next->s_procQ);
    printf("semdh sprocQ letizia: %d\n",semd_h->s_next->s_procQ->p_next);

    printf("\nprnt: %d child: %d\n", asdrubale->p_prnt, asdrubale->p_child);

    /*printf("\nprova remove: %d", removeBlocked(&semadd));
    printf("\nprova remove2: %d", removeBlocked(&semadd));
    printf("\nprova remove3: %d", removeBlocked(&semadd));*/

    printf("\nheadBolcked sem1: %d", headBlocked(&semadd) );
    printf("\nheadBolcked sem2: %d", headBlocked(&semadd2) );

    printf ("prova outbolcked1: %d", outBlocked(leon));

    printf("\nheadBolcked sem1: %d", headBlocked(&semadd) );
    printf("\nheadBolcked sem2: %d", headBlocked(&semadd2) );

    printf("\nASL ( %d ) : %d",  *(semd_h->s_semAdd) ,semd_h);
    printf("\nASL next ( %d ) : %d",*(semd_h->s_next->s_semAdd) , semd_h->s_next);
    printf("\nleon: %d, sem next: %d", leon, semd_h->s_next->s_procQ);




    return 0;
}//end main

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
void insertProcQ(pcb_t** tp, pcb_t* p) {

    pcb_PTR sent_tp = *tp;

    if (sent_tp != NULL)
    {
        pcb_PTR tmp = sent_tp->p_prev;
        //assegnamento al successivo dell'ultimo a p
        tmp->p_next = p;
        //assegnamento al successivo del nuovo ultimo il primo elemento della lista
        p->p_next = sent_tp;
        //il precedente del primo è p
        sent_tp->p_prev = p;
        //il precedente di p è l'ultimo prima dell'inserimento di p
        p->p_prev = tmp;
    }else{
        sent_tp = p;
        p->p_next = p;
        p->p_prev = p;
    }
    *tp=sent_tp;
}

/*Restituisce l’elemento in fondo alla coda (ultimo immesso) dei processi tp, SENZA RIMUOVERLO.
Ritorna NULL se la coda non ha elementi.*/
pcb_t* headProcQ(pcb_t** tp){
    pcb_PTR sent = *tp;

    if (sent == NULL)
        return NULL;
    else
        return sent->p_prev;
}

/*Rimuove l’elemento piu’ vecchio dalla coda tp. Ritorna NULL se la coda è vuota, altrimenti ritorna il puntatore all’elemento
rimosso dalla lista.*/
pcb_t* removeProcQ(pcb_t **tp){
    //primo elemento sentinella
    pcb_PTR sent_tp = *tp;
    pcb_PTR elem_toremove = *tp;

    if(*tp == NULL)
        return NULL;
    else if(sent_tp->p_next == sent_tp){

        *tp=NULL;
        return elem_toremove;
    }else{
        //ultimo elemento
        pcb_PTR tmp=sent_tp->p_prev;

        //rimuovo l'elemento piu vecchio della coda tp ovvero la sentinella
        //sentinella diventa il secondo elemento
        sent_tp=sent_tp->p_next;
        //il precedente della nuova sentinella diventa l'ultimo elemento
        sent_tp->p_prev=tmp;
        //l'ultimo elemento punta alla nuova sentinella
        tmp->p_next=sent_tp;

        *tp=sent_tp;
        return elem_toremove;

        }
}

/*Rimuove il PCB puntato da p dalla coda dei processi puntata da tp. Se p non è presente nella coda, restituisce NULL (p può trovarsi in una posizione arbitraria della coda).*/
pcb_t* outProcQ(pcb_t **tp, pcb_t *p) {
    //primo elemento sentinella
    pcb_PTR sent_tp = *tp;

    //puntatore all'elemento da rimuovere
    pcb_PTR elem_toremove = NULL;

    if (*tp == NULL) {
        return NULL;
    }

    else{
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

                //se l'elemento da rimuovere è la sentinella ovvero il primo elemento
                if (elem_toremove == sent_tp)
                    *tp = elem_toremove->p_next;
            }
        }
    return elem_toremove;
    }
}

/*Restituisce TRUE se il PCB puntato da p non ha figli, FALSE altrimenti. */
int emptyChild(pcb_t *p){

    if(p->p_child == NULL)
        return 1;

    return 0;
}

/*Inserisce il PCB puntato da p come figlio del PCB puntato da prnt.*/
void insertChild(pcb_t *prnt, pcb_t *p){

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

/*Viene inserito il PCB puntato da p nella coda deiprocessi bloccati associata al SEMD con chiave semAdd. Se il semaforo corrispondente non è presente nella ASL,
 * alloca un nuovo SEMD dalla lista di quelli liberi (semdFree) e lo inserisce nella ASL, settando I campi in maniera opportuna (i.e.
key e s_procQ). Se non è possibile allocare un nuovo SEMD perché la lista di quelli liberi è vuota, restituisce TRUE. In tutti gli altri casi, restituisce FALSE. */
int insertBlocked(int *semAdd,pcb_t *p){
    semd_PTR tmp = semd_h;

    //se la lista dei semafori attivi è vuota
    if(tmp == NULL){
        //alloco il primo semaforo
        semd_PTR newsem = semdFree_h;
        semdFree_h = semdFree_h->s_next;

        newsem->s_next = NULL;
        newsem->s_procQ = mkEmptyProcQ();
        newsem->s_semAdd = semAdd;

        //aggiungo il semaforo nella lista dei semafori attivi ASL
        semd_h = newsem;
        p->p_semAdd = semAdd;
        //inserisco p nel nuovo semaforo
        insertProcQ( &( newsem->s_procQ ), p );
        return 0;
    }

    //se la lista dei semafori attivi è piena (ovvero lista semafori liberi vuota)
    else if (semdFree_h == NULL) {
        return 1;
    }

    //se la lista sei semafori attivi contiene almeno un elemento ma non è piena
    else{

        //cerco il semaforo nella lista dei semafori allocati
        while(tmp != NULL){

            if( *(tmp->s_semAdd) == *semAdd ){
                p->p_semAdd= semAdd;
                insertProcQ(&(tmp->s_procQ),p);
                return 0;
            }

            tmp=tmp->s_next;

        }
        //se non ho trovato il semaforo tra i semafori allocati allora aggiungo un semaforo
        //alloco un nuovo semaforo
        semd_PTR newsem=semdFree_h;
        semdFree_h=semdFree_h->s_next;

        newsem->s_next=semd_h;
        newsem->s_procQ=mkEmptyProcQ();
        newsem->s_semAdd=semAdd;
        //aggiungo il semaforo nella lista dei semafori attivi ASL
        semd_h=newsem;
        //inserisco p nel nuovo semaforo
        p->p_semAdd= semAdd;
        insertProcQ(&(newsem->s_procQ),p);

        return 0;
    }


}


/*Ritorna il primo PCB dalla coda dei processi bloccati (s_procq) associata al SEMD della ASL con chiave semAdd. Se tale descrittore non esiste nella ASL,
 * restituisce NULL. Altrimenti, restituisce l’elemento rimosso. Se la coda dei processi bloccati per il semaforo diventa vuota, rimuove il descrittore
corrispondente dalla ASL e lo inserisce nella coda dei descrittori liberi (semdFree_h). */

//PROCQ PUNTA ALLA TESTA DELLA CODA E NON ALLA CODA DELLA CODA !!!!!!!!!!!!!!!!!!!!!!!!

pcb_t* removeBlocked(int *semAdd){

    //se la lista di semafori attivi è vuota, non ho niente da rimuovere
    if(semd_h == NULL){
          return NULL;

    }else{ //se la lista di semafori attivi ha almeno un elemento
        pcb_PTR elem_toremove = NULL;
        semd_PTR tmp = semd_h;

        semd_PTR old_tmp = NULL;
        int found = 0;

        while(tmp != NULL && found == 0){
            if(*(tmp->s_semAdd) == *semAdd){ //ho trovato il semaforo
                found = 1;
                if(tmp->s_procQ->p_next == tmp->s_procQ){ // caso in cui c'è un solo elem da rimuovere, sposto codesto semaforo tra quelli liberi
                        if(old_tmp == NULL){ //se il puntatore al semaforo precedente è null ovvero il samaforo trovato è il primo,  rimuoviamo il semaforo e spostiamo la sentinella
                            semd_h=tmp->s_next;
                            tmp->s_next= semdFree_h;
                            semdFree_h=tmp;

                        }else{
                            old_tmp->s_next=tmp->s_next; //togliamo il semaforo  dalla lista di semafori bloccati (asl)
                            tmp->s_next=semdFree_h; //aggiungiamo il semaforo alla lista di semafori liberi
                            semdFree_h=tmp;
                        }
                }
                elem_toremove=removeProcQ(&(tmp->s_procQ));
                outChild(elem_toremove);
            }
            old_tmp=tmp;
            tmp=tmp->s_next;
        }

        //se non lo trovo
        if (found == 0)
            return NULL;

        return elem_toremove;
  }
}

/*
 * Rimuove il PCB puntato da p dalla coda delsemaforo su cui è bloccato (indicato da p->p_semAdd). Se il PCB non compare in tale coda, allora restituisce NULL (condizione di errore).
Altrimenti, restituisce p.  */
pcb_t* outBlocked(pcb_t *p){
    if(p->p_semAdd == NULL) //se il semaforo di p non esiste return NULL
        return NULL;

    else{ //se il semaforo di p esiste
        semd_PTR tmp = semd_h;
        semd_PTR old_tmp = NULL;
        pcb_PTR elem_toremove = NULL;
        int found = 0;

        while(tmp != NULL && found == 0){ //cerco il semaforo di p nella lista di semafori attivi (ASL)
            if(*(tmp->s_semAdd) == *(p->p_semAdd))
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
}

/*Restituisce (senza rimuovere) il puntatore al PCB che si trova in testa alla coda dei processiassociata al SEMD con chiave semAdd. Ritorna NULL se il SEMD non compare nella ASL oppure
se compare ma la sua coda dei processi è vuota.*/
pcb_t* headBlocked(int *semAdd){
    semd_PTR tmp = semd_h;

    while(tmp != NULL ){
        if(*(tmp->s_semAdd) == *(semAdd)){
            /*if(tmp->s_procQ == NULL) {return ERRORE MOOOOOOOOOOOOLTO GRAVE (LETIZIA);}*/
            return headProcQ( &(tmp->s_procQ) );
        }
        tmp = tmp->s_next;
    }
    return NULL;
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
    //creo sentinella
    semdFree_h = & semd_table[0];

    for ( i=0; i<MAXPROC ; i++ ) {
        if(i == 0)//primo semaforo
            semd_table[i].s_next = & semd_table[i+1];

        else if(i == MAXPROC-1)//ultimo semaforo
            semd_table[i].s_next = & semd_table[0];

        else
            semd_table[i].s_next = & semd_table[i+1];
    }
}
