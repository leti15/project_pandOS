#include <stdio.h>
#include <stdlib.h>

#include "pcb.h"
#include "asl.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "p2test.c"
#define STATE_INIT 0b00011000000000000000000000001100
#define STATE_WAIT 0b00010000000000000000000000000001

void scheduler(){
    current_proc = removeProcQ(readyQ);
    //carica 5 millisecondi in PLT
    setTIMER(5000 *(*((cpu_t *) TIMESCALEADDR)));
    LDST(&(current_proc->p_s));

    //in base a com'è la readyQ ci comportiamo in modo diverso
    if (emptyProcQ(readyQ) == 1)//se è vuota
    {

        /** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!s
         * siccome le macro HALT, WAIT e PANIC richiedono 
         * la kernel mode dovremmo settare almeno quel bit ad 1...come fare? 
         * NB: per il caso WAIT è già sistemato
        */
        if (proc_count == 0){   HALT();}
        if (proc_count > 0 & softB_count > 0){  
            //disabilitare PLT e abilitare interrupts
            setSTATUS(STATE_WAIT);
            WAIT();
        }
        if (proc_count > 0 & softB_count == 0){ PANIC();}
    }
}