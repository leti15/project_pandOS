#include "pcb.h"
#include "asl.h"
#include "pandos_types.h"
#include "commons.h"
#include "exceptions_handler.h"
#include "scheduler.h"

void scheduler()
{   
    sysSCHEDULER();

    current_proc = removeProcQ(&readyQ);
    if (current_proc == NULL){//se la readyQ è vuota

        if (proc_count == 0){ HALT(); }
        if (proc_count > 0 && softB_count > 0){  
            //disabilitare PLT e abilitare interrupts
            setSTATUS(STATE_WAIT);
            WAIT();
        }
        if (proc_count > 0 && softB_count == 0){ PANIC(); }
    }else{
        
        //carica 5 millisecondi in PLT
        setTIMER(5000 *(*((cpu_t *) TIMESCALEADDR)));
        *((int*)TODLOADDR) = 0;
        LDST( (state_t*) &(current_proc->p_s));
    }

}