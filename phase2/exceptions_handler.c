#include "commons.h"
#include "exceptions_handler.h"

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


/*
 * #define CAUSE_GET_EXCCODE(x)   (((x) & CAUSE_EXCCODE_MASK) >> CAUSE_EXCCODE_BIT)
 * Il valore di E1 << E2 è E1 spostato a sinistra di E2 posizioni di bit. 
 * 
 * I bit vuoti vengono riempiti con zero. Se E1 dispone di un tipo senza segno, 
 * il valore del risultato è E1 × 2^E2, modulo ridotto uno più del valore massimo 
 * rappresentabile nel tipo di risultato. In caso contrario, se E1 ha un tipo con segno 
 * e un valore non negativo e E1 × 2^E2 è rappresentabile nel tipo senza segno 
 * corrispondente del tipo di risultato, tale valore, convertito nel tipo di risultato, 
 * è il valore risultante; in caso contrario, il comportamento non è definito.
 *
 * Il valore di E1 >> E2 è E1 spostato a destra di E2 posizioni di bit.
 * Se E1 ha un tipo senza segno o se E1 ha un tipo con segno e un valore non negativo, 
 * il valore del risultato è la parte integrale del quoziente di E1/2E2. Se E1 dispone di un 
 * tipo signed e di un valore negativo, il valore risultante sarà definito 
 * dall'implementazione
 */
    unsigned int current_causeCode = getCAUSE();
    int exCode = CAUSE_GET_EXCCODE(current_causeCode); 
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
/** 
 *  "per convenzione il processo in esecuzione posiziona appropriati valori
 *  nei registri a0, a1, a2, a3 (corrispondenti a gpr[3], gpr[4], gpr[5], gpr[6])
 *  subito prima di eseguire l'istruzione syscall"
 *                              -cit. Davoli in pandos.pdf pg 25 (35)
 *  Quando si è in kernel mode guardo il registro a0 = gpr[3] 
 *  per capire quale syscall eseguire
 * */    
    int current_a0 = current_proc->p_s.gpr[3];
    if (current_a0 == 1){
        //create process
        SYSCALL(CREATEPROCESS, &current_proc->p_s, current_proc->p_supportStruct, 0);

    }else if (current_a0 == 2){
        //terminate process
        SYSCALL(TERMPROCESS, 0, 0, 0);

    }else if (current_a0 == 3){
        //passeren
        SYSCALL(PASSEREN, current_proc->p_semAdd, 0, 0);

    }else if (current_a0 == 4){
        //verhogen
        SYSCALL(VERHOGEN, current_proc->p_semAdd, 0, 0);

    }else if (current_a0 == 5){
        //wait for IO
        int intlNo = current_proc->p_s.gpr[4];//numero linea
        int dnum = current_proc->p_s.gpr[5];//numero device 
        int termRead = current_proc->p_s.gpr[6];//se = 1 è un terminale in lettura
        SYSCALL(IOWAIT, intlNo, dnum, termRead);

    }else if (current_a0 == 6){
        //get CPU time
        SYSCALL(GETCPUTIME, 0, 0, 0);

    }else if (current_a0 == 7){
        //wait for clock
        SYSCALL(WAITCLOCK, 0, 0, 0);

    }else if (current_a0 == 8){
        //get support data
        support_t* p_support;
        p_support = SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    }
}

void trap_handler(){


}

void interrupt_handler(){



}
