#include "commons.h"
#include "scheduler.h"
#include "exceptions_handler.h"

/*

#define STATE_INIT 0b00011000000000000000000000001100
#define STATE_WAIT 0b00010000000000000000000000000001

//macro to find old kernel/user mode
#define STATUSO_MODE_MASK 0b00000000000000000000000000100000
#define STATUSO_MODE_BIT 5
#define STATUSO_GET_MODE(x)   (((x) & STATUSO_MODE_MASK) >> STATUSO_MODE_BIT)

//macro to find previous kernel/user mode
#define STATUSP_MODE_MASK 0b00000000000000000000000000001000
#define STATUSP_MODE_BIT 3
#define STATUSP_GET_MODE(x)   (((x) & STATUSP_MODE_MASK) >> STATUSP_MODE_BIT)

//macro to find current kernel/user mode
#define STATUSC_MODE_MASK 0b00000000000000000000000000000010
#define STATUSC_MODE_BIT 1
#define STATUSC_GET_MODE(x)   (((x) & STATUSC_MODE_MASK) >> STATUSC_MODE_BIT)
*/


int proc_count;
HIDDEN int softB_count;
HIDDEN pcb_PTR readyQ;
HIDDEN pcb_PTR current_proc;
HIDDEN passupvector_t PassUpVector [16];
HIDDEN passupvector_t* PUV;
/** puntatore alla coda dei semafori attivi 'semd_h' */

//device semaphores
/**
 * 0-7: DEVICE LINEA 3
 * 8-15:DEVICE LINEA 4
 * 16-23: DEVICE LINEA 5
 * 24-31: DEVICE LINEA 6
 * 32-47: DEVICE LINEA 7 (TERMINALI)
 * 48: DEVICE INTERVAL TIMER
 * 49: DEVICE PLT
*/
HIDDEN semd_t* device[50]; 

void init_devices() {
    //costruzione bit map 
    //DA RIGUARDARE
    int mask, j;
    unsigned int* x = 0x1000002C; 

    for (int i=0; i<4; i++) {
        mask = 1;
        for(j=0; j<8; j++) {
            if(*x & mask > 0) {
                
            }
        }
    }

    //costruzione bit map linea 7
}

extern void fooBar();
extern void test();
extern void SYS_handler();
extern void trap_handler();
extern void interrupt_handler();

int main()
{

/**  Da fare:
*    - Inizializzare i moduli di fase 1 ( initPcbs() e initSemd() )
*    - Inizializzare le variabili di cui sopra
*    - Popolare il pass up vector con gestore e stack pointer per eccezioni e TLB-Refill
*/

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
