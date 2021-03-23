#include <stdio.h>
#include <stdlib.h>

#include "pcb.h"
#include "asl.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "p2test.c"
#include "scheduler.c"

#define STATE_INIT 0b00011000000000000000000000001100
#define STATE_WAIT 0b00010000000000000000000000000001

HIDDEN int proc_count;
HIDDEN int softB_count;
HIDDEN pcb_PTR readyQ;
HIDDEN pcb_PTR current_proc;
HIDDEN passupvector_t PassUpVector [16];
HIDDEN passupvector_t* PUV;
/** puntatore alla coda dei semafori attivi 'semd_h' */

void fooBar(){};
extern void test();
void SYS_handler();
void trap_handler();
void interrupt_handler();

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

void foobar(){
/** in base al 'cause_register' pg.18 di MPS3 e di preciso al campo 'ExcCode'
 *  distinguiamo i diversi interrupt
0 Int = External Device Interrupt
1 Mod = TLB-Modification Exception
2 TLBL = TLB Invalid Exception: on a Load instr. or instruction fetch
3 TLBS = TLB Invalid Exception: on a Store instr.
4 AdEL = Address Error Exception: on a Load or instruction fetch
5 AdES = Address Error Exception: on a Store instr.
6 IBE = Bus Error Exception: on an instruction fetch
7 DBE = Bus Error Exception: on a Load/Store data access
8 Sys = Syscall Exception
9 Bp = Breakpoint Exception
10 RI = Reserved Instruction Exception
11 CpU = Coprocessor Unusable Exception
12 OV = Arithmetic Overflow Exception
*/

    unsigned int current_causeCode = getCAUSE();
    int exCode;
    //in base all' exCode capisco cosa fare
    if (exCode == 0){
        //External Device Interrupt
        interrupt_handler();
    }else if(exCode >=1 && exCode <= 3){
        //eccezioni TLB
        uTLB_RefillHandler();
    }else if( (exCode > 3 && exCode <= 7) || (exCode > 8 && exCode <= 12) ){
        //program trap exception handler
        trap_handler();
    }else if(exCode == 8){
        //system calls
        SYS_handler();
    }
}

void SYS_handler(){
//se si è in kernel mode guardo il registro a0 = gpr[3]
    if (current_proc->p_s.gpr[3] == 1 ){
        //create process
        SYSCALL(CREATEPROCESS, &current_proc->p_s, current_proc->p_supportStruct, 0);

    }else if (current_proc->p_s.gpr[3] == 2 ){
        //terminate process
        SYSCALL(TERMPROCESS, 0, 0, 0);

    }else if (current_proc->p_s.gpr[3] == 3 ){
        //passeren
        SYSCALL(PASSEREN, current_proc->p_semAdd, 0, 0);

    }else if (current_proc->p_s.gpr[3] == 4 ){
        //verhogen
        SYSCALL(VERHOGEN, current_proc->p_semAdd, 0, 0);

    }else if (current_proc->p_s.gpr[3] == 5 ){
        //wait for IO
        int intlNo;//numero linea
        int dnum;//numero device 
        int termRead;//se = 1 è un terminale in lettura
        SYSCALL(IOWAIT, intlNo, dnum, termRead);

    }else if (current_proc->p_s.gpr[3] == 6 ){
        //get CPU time
        SYSCALL(GETCPUTIME, 0, 0, 0);

    }else if (current_proc->p_s.gpr[3] == 7 ){
        //wait for clock
        SYSCALL(WAITCLOCK, 0, 0, 0);

    }else if (current_proc->p_s.gpr[3] == 8 ){
        //get support data
        support_t* p_support;
        p_support = SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    }
}

void trap_handler(){


}

void interrupt_handler(){



}
