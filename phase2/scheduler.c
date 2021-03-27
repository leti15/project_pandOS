#include "commons.h"
#include "scheduler.h"

void scheduler()
{    
    //in base a com'è la readyQ ci comportiamo in modo diverso
    if (emptyProcQ(readyQ) == 1)//se è vuota
    {
        if (proc_count == 0){   HALT();}
        if (proc_count > 0 & softB_count > 0){  
            //disabilitare PLT e abilitare interrupts
            setSTATUS(STATE_WAIT);
            WAIT();
        }
        if (proc_count > 0 & softB_count == 0){ PANIC();}
    }

    current_proc = removeProcQ(readyQ);
    
    //carica 5 millisecondi in PLT
    setTIMER(5000 *(*((cpu_t *) TIMESCALEADDR)));
    LDST(&(current_proc->p_s));

    //come eseguirlo??????????????????????????

}