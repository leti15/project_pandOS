#include <stdio.h>
#include <stdlib.h>

#define MAXPROC 20

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

//controllare se serve
int n_pcb_free;

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

//main
int main(){

    initPcbs();
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
*/

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
        pcbFree_table[i].p_next=NULL;
        pcbFree_table[i].p_prev=NULL;
        pcbFree_table[i].p_prnt=NULL;
        pcbFree_table[i].p_child=NULL;
        pcbFree_table[i].p_next_sib=NULL;
        pcbFree_table[i].p_prev_sib=NULL;
        //pcbFree_table[i].p_s=
        //pcbFree_table[i].p_time=
        pcbFree_table[i].p_semAdd=NULL;
    }

    //creo sentinella
    pcbFree_h=&pcbFree_table[0];


    for (i=0; i<MAXPROC ;i++) {
        if(i==0){//primo pcb
            pcbFree_table[i].p_next=&pcbFree_table[i+1];
            pcbFree_table[i].p_prev=&pcbFree_table[MAXPROC-1];

        }
        else if(i==MAXPROC-1){//ultimo pcb
            pcbFree_table[i].p_next=&pcbFree_table[0];
            pcbFree_table[i].p_prev=&pcbFree_table[i-1];
        }
        else {
                pcbFree_table[i].p_next=&pcbFree_table[i+1];
                pcbFree_table[i].p_prev=&pcbFree_table[i-1];
        }
    }
    //CONTROLLARE SE SERVE
    n_pcb_free=MAXPROC;
}

/* inserisce il PCB puntato da p nella lista dei PCB liberi (pcbFree_h)*/

void freePcb(pcb_t *p){

    //controlliamo che sia possibile aggiungere un pcb
    if (n_pcb_free==MAXPROC){
        printf("ERROR: Non è possibile aggiungere un pcb!!!!!");
    }
    else if (n_pcb_free==0){
        p->p_next=p;
        p->p_prev=p;
        pcbFree_h=p;
        n_pcb_free=1;
    }
    else{
        //proviamo a fare inserimento in coda (da controllare)
        //il "prev" del primo elemento diventerà p
        //il "next" dell'ultimo elemento diventerà p
        pcb_PTR temp=pcbFree_h->p_prev; //temp è l'ultimo pcb
        pcbFree_h->p_prev=p; //il primo punterà al nuovo p
        p->p_prev=temp;//il NUOVO ultimo (=p) avrà come precedente il VECCHIO ultimo (=temp)
        temp->p_next=p; //il VECCHIO ultimo avrà come successivo il NUOVO ultimo
        p->p_next=pcbFree_h; // il NUOVO ultimo avrà come successore la sentinella quindi il primo pcb

        n_pcb_free=n_pcb_free+1;
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
    pcb_PTR tmp;
    tmp= NULL;
    return tmp;
}
/*Restituisce TRUE se la lista puntata da head è vuota, FALSE altrimenti*/
int emptyProcQ(pcb_t* tp) {
    if (tp == NULL){
        return 1;}
    else{
        return 0;}
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
    }
    else
    {
        sent_tp = p;
        p->p_next = p;
        p->p_prev = p;
    }
    *tp=sent_tp;
}

/*Restituisce l’elemento in fondo alla coda dei processi tp, SENZA RIMUOVERLO.
Ritorna NULL se la coda non ha elementi.*/

pcb_t* headProcQ(pcb_t** tp){
    pcb_PTR sent = *tp;

    if (sent == NULL){
        return NULL;
    }
    else{
        return sent->p_prev;
    }
}

pcb_t* removeProcQ(pcb_t **tp){
    //primo elemento sentinella
    pcb_PTR sent_tp= *tp;
    pcb_PTR elem_toremove=*tp;

    if(*tp == NULL){
        return NULL;
                        }
    else if(sent_tp->p_next==sent_tp){
        *tp=NULL;
        return elem_toremove;
    }
    else{
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
pcb_t* outProcQ(pcb_t **tp, pcb_t *p) {
    //primo elemento sentinella
    pcb_PTR sent_tp = *tp;

    //puntatore all'elemento da rimuovere
    pcb_PTR elem_toremove=NULL;

    if (*tp == NULL) {


        return NULL;
    }
    else{
        int found =0;
        pcb_PTR tmp=sent_tp;

        do{
                if(tmp == p){
                found=1;
                elem_toremove=tmp;
                }
                tmp=tmp->p_next;

        }while(found==0 && tmp!=sent_tp);

        if(found==1)
        {
            //caso in cui elemento da rimuovere sia uno solo
            if (elem_toremove->p_prev == elem_toremove) {
                *tp = NULL;
            }
                //caso in cui l'elemento è in mezzo ad altri elementi
            else {
                //se l'elemento da rimuovere è la sentinella ovvero il primo elemento
                elem_toremove->p_prev->p_next = elem_toremove->p_next;
                elem_toremove->p_next->p_prev = elem_toremove->p_prev;
                if (elem_toremove == sent_tp) {
                    *tp = elem_toremove->p_next;
                }


            }
        }
    return elem_toremove;
    }

}
/*Restituisce TRUE se il PCB puntato da p non ha figli, FALSE altrimenti. */
int emptyChild(pcb_t *p){

    if(p->p_child == NULL){
        return 1;
    }
    return 0;
}

/*Inserisce il PCB puntato da p come figlio del PCB puntato da prnt.*/
void insertChild(pcb_t *prnt, pcb_t *p){

    if(prnt->p_child != NULL){

        //troviamo il ptb esistente che punterà al nuovo ptb
        pcb_PTR last=prnt->p_child->p_prev_sib;

        //l'ultimo figlio punta al nuovo figlio
        last->p_next_sib=p;
        p->p_prev_sib=last;
        p->p_prnt=prnt;
        p->p_next_sib=prnt->p_child;
        prnt->p_child->p_prev_sib=p;

    }

    else{
        prnt->p_child=p;
        p->p_prnt=prnt;

        p->p_prev_sib=p;
        p->p_next_sib=p;

    }
}

/*Rimuove il primo figlio del PCB puntato da p. Se p non ha figli, restituisce NULL.*/
pcb_t* removeChild(pcb_t *p){

    if(p->p_child == NULL){
        return NULL;
    }
    else if(p->p_child->p_prev_sib == p->p_child){
        pcb_PTR elem_toremove=p->p_child;
        p->p_child=NULL;

        //pulisco il puntatore all'elemento rimosso
        elem_toremove->p_next_sib=NULL;
        elem_toremove->p_prev_sib=NULL;
        elem_toremove->p_prnt=NULL;
        return elem_toremove;
        }
    else{
        pcb_PTR elem_toremove=p->p_child;
        pcb_PTR last=p->p_child->p_prev_sib;
        //collego il secondo elemento con l'ultimo
        last->p_next_sib=p->p_child->p_next_sib;
        p->p_child->p_next_sib->p_prev_sib=last;

        //collego il padre con il nuovo primo figlio
        p->p_child = p->p_child->p_next_sib;

        //pulisco il puntatore all'elemento rimosso
        elem_toremove->p_next_sib=NULL;
        elem_toremove->p_prev_sib=NULL;
        elem_toremove->p_prnt=NULL;
        return elem_toremove;
    }

}




/*Rimuove il PCB puntato da p dalla lista dei figli del padre. Se il PCB puntato da p non ha un padre, restituisce NULL,
altrimenti restituisce l’elemento rimosso (cioè p). A differenza della removeChild, p può trovarsi in una posizione arbitraria (ossia non è
necessariamente il primo figlio del padre).*/

pcb_t *outChild(pcb_t* p){
    //se p non ha un padre
    if(p->p_prnt == NULL){
        return NULL;
    }

    //se p è l'unico figlio oppure è il primo figlio
    else if(p->p_next_sib == p || p->p_prnt->p_child == p){
        return removeChild(p->p_prnt);

    }

    else{
        p->p_prev_sib->p_next_sib=p->p_next_sib;
        p->p_next_sib->p_prev_sib=p->p_prev_sib;

        //pulisco il puntatore all'elemento rimosso
        p->p_next_sib=NULL;
        p->p_prev_sib=NULL;
        p->p_prnt=NULL;
        return p;

    }

}