#include "commons.h"
#include "scheduler.h"
#include "exceptions_handler.h"

int proc_count;
int softB_count;
pcb_PTR readyQ;
pcb_PTR current_proc;
passupvector_t* PUV;
semd_t* device[DEVARRSIZE];
// puntatore alla coda dei semafori attivi 'semd_h'

int main()
{
    proc_count = 0;
    softB_count = 0;
    readyQ = mkEmptyProcQ();
    current_proc = NULL;
    
    PUV->exception_handler = (memaddr) fooBar;
    PUV->exception_stackPtr = 0x20001000;
    PUV->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    PUV->tlb_refill_stackPtr = 0x20001000;

    initPcbs();
    initASL();
    init_devices();

    LDIT(100000);  // setto interval timer di 100 millisecondi (=100000 microsecondi)
    
    // inizializzo il primo processo
    pcb_PTR p = allocPcb();
    p->p_s.status = STATE_INIT; 
    p->p_time = 0;
    p->p_s.pc_epc = (memaddr) test; // PC set to the address of test()
    p->p_s.gpr[24] = (memaddr) test;
    RAMTOP(p->p_s.gpr[26]); // SP set to RAMTOP?
    insertProcQ(&readyQ, p);

    //chiama lo scheduler
    scheduler();
    


    return 0;
}
