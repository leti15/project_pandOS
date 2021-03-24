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
    //e' in kernel mode con interrupt disabilitati
    unsigned int* status_reg = (unsigned int*) BIOSDATAPAGE;
    current_proc->p_s.status = status_reg;

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
    if( STATUSC_GET_MODE(current_proc->p_s.status) == 1) {
        //il processo va ucciso perchè era in user mode



    }else{
        int current_a0 = current_proc->p_s.gpr[3];
        if (current_a0 == 1){
            //create process    SYSCALL(CREATEPROCESS, &current_proc->p_s, current_proc->p_supportStruct, 0);
            pcb_PTR newProc = allocPcb();
            if (newProc == NULL){ 
                //non ci sono pcb liberi= mancanza risorse
                current_proc->p_s.gpr[1] = -1;
            }
            proc_count = proc_count + 1;
            insertChild(current_proc, newProc);
            newProc->p_time = 0;
            newProc->p_supportStruct = current_proc->p_s.gpr[5];
            state_t* temp = current_proc->p_s.gpr[4];
            newProc->p_s = *temp;
            insertProcQ(&readyQ, newProc);
            current_proc->p_s.gpr[1] = 0;

        }else if (current_a0 == 2){
            //terminate process SYSCALL(TERMPROCESS, 0, 0, 0);

            pcb_PTR tempQueue = mkEmptyProcQ(); //frangia dei pcb non ancora terminati 
            pcb_PTR temp = current_proc;
            outChild(current_proc);
            insertProcQ(&tempQueue, current_proc);
            //emptyChild == 1 se NON ha figli

            while (emptyProcQ(&tempQueue) != 1) //finchè non è vuota continuo 
            {
                pcb_PTR to_terminate = removeProcQ(&tempQueue);

                while(emptyChild(to_terminate) != 1)
                {   //se ha figli
                    pcb_PTR child_toRemove = removeChild(to_terminate);
                    insertProcQ(&tempQueue, child_toRemove);
                }
                //a questo punto 'to_terminate' non avrà più figli e lo posso terminare
                if (to_terminate->p_semAdd != NULL){ outBlocked(to_terminate);   }
                outProcQ(&readyQ, to_terminate);
                freePcb(to_terminate);
                proc_count = proc_count - 1;
            }

            //chiamimamo lo scheduler
            scheduler();

        }else if (current_a0 == 3){
            //passeren            SYSCALL(PASSEREN, current_proc->p_semAdd, 0, 0);



        }else if (current_a0 == 4){
            //verhogen  SYSCALL(VERHOGEN, current_proc->p_semAdd, 0, 0);



        }else if (current_a0 == 5){
            //wait for I    SYSCALL(IOWAIT, intlNo, dnum, termRead);



        }else if (current_a0 == 6){
            //get CPU time  SYSCALL(GETCPUTIME, 0, 0, 0);



        }else if (current_a0 == 7){
            //wait for clock    SYSCALL(WAITCLOCK, 0, 0, 0);



        }else if (current_a0 == 8){
            //get support data     p_support = SYSCALL(GETSUPPORTPTR, 0, 0, 0);
        
        
        
        }  

        //incremento il PC di una word (= 4) in caso sia una delle SYSCALL che non blocca il processo
        if (current_a0 == 1 ||current_a0 == 2 || current_a0 == 4 || current_a0 == 6 || current_a0 == 8){
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc  + 4; 
        }  
    }
    
}

void trap_handler(){


}

void interrupt_handler(){



}
