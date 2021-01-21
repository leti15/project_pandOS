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
    /* state_t p_s;  processor state */
     /* cpu_t p_time; cpu time used by proc */
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


int main()
{

    initPcbs();
    for (int i=0; i<MAXPROC;i++){
        printf("\nfreeTab (%d): %d", i, &pcbFree_table[i]);

    }



    pcb_t prova;
    printf("\n\n ind prova: %d\n",&prova);
    freePcb(&prova);

    pcb_PTR temp=pcbFree_h;
    for(int i=0; i<=n_pcb_free; i++){

        printf("\n\npcb(%d): %d", i, temp);
        printf("\tind_next:%d \ncont_next: %d", (temp->p_next), *(temp->p_next) );
        printf("\tind_prev:%d \ncont_prev: %d", (temp->p_prev), *(temp->p_prev) );
        temp=temp->p_next;
    }

    return 0;
}


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


    for (i=0; i<MAXPROC ;i++){
        if(i==0){//primo pcb
            pcbFree_table[i].p_next=&pcbFree_table[i+1];
            pcbFree_table[i].p_prev=&pcbFree_table[MAXPROC-1];

        }
        else if(i==MAXPROC-1){//ultimo pcb
            pcbFree_table[i].p_next=&pcbFree_table[0];
            pcbFree_table[i].p_prev=&pcbFree_table[i-1];
        }
        else{
                pcbFree_table[i].p_next=&pcbFree_table[i+1];
                pcbFree_table[i].p_prev=&pcbFree_table[i-1];
        }
    }
    //CONTROLLARE SE SERVE
    n_pcb_free=MAXPROC;
}
void freePcb(pcb_t *p){

    //controlliamo che sia possibile aggiungere un pcb
    if (n_pcb_free==MAXPROC){
        printf("!!!!!!ERROR!!!!!");
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

    //resettare il pcb p
    p->p_child=NULL;
    p->p_next_sib=NULL;
    p->p_prev_sib=NULL;
    p->p_prnt=NULL;
    p->p_semAdd=NULL;

}
pcb_t *allocPcb(){}
pcb_t* mkEmptyProcQ(){}
int emptyProcQ (pcb_t *tp){}
