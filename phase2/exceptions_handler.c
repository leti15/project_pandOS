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
    state_t* state_reg = BIOSDATAPAGE;
    current_proc->p_s = state_reg;

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
        SYSCALL(TERMPROCESS, 0, 0, 0);

    }else{
        int current_a0 = current_proc->p_s.gpr[3];
        if (current_a0 == 1){
            //create process    SYSCALL(CREATEPROCESS, &current_proc->p_s, current_proc->p_supportStruct, 0);
            if (current_proc->p_s.gpr[4] == NULL){ SYSCALL(TERMPROCESS, 0, 0, 0); } 
            
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
            //INCREMENTIAMO IL PC 
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc  + 4; 

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
            if (current_proc->p_s.gpr[4] == NULL){ SYSCALL(TERMPROCESS, 0, 0, 0); }
            
            int* temp = current_proc->p_s.gpr[4];
            *temp = *temp - 1;
            //INCREMENTIAMO IL PC 
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc  + 4; 
            if (*temp < 0) {
                //INCREMENTIAMO IL PC 
                unsigned int tmp = STCK(tmp);
                current_proc->p_time = current_proc->p_time + tmp;
                insertBlocked(temp, current_proc);
                scheduler();
            }          

        }else if (current_a0 == 4){
            //verhogen  SYSCALL(VERHOGEN, current_proc->p_semAdd, 0, 0);
            if (current_proc->p_s.gpr[4] == NULL){ SYSCALL(TERMPROCESS, 0, 0, 0); }
            
            int* temp = current_proc->p_s.gpr[4];
            *temp = *temp + 1;
            //INCREMENTIAMO IL PC 
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc  + 4; 
            //controllare se dobbiamo risvegliare il processo o no

        }else if (current_a0 == 5){
            //wait for IO    SYSCALL(IOWAIT, intlNo, dnum, termRead);
            if (current_proc->p_s.gpr[4] == NULL || current_proc->p_s.gpr[5] == NULL || current_proc->p_s.gpr[6] == NULL){ return NULL; }
            
            /**
             * 0-7: DEVICE LINEA 3
             * 8-15:DEVICE LINEA 4
             * 16-23: DEVICE LINEA 5
             * 24-31: DEVICE LINEA 6
             * 32-47: DEVICE LINEA 7 (TERMINALI)
             *      32-39: TERMINALI IN SCRITTURA
             *      40-47: TERMINALI IN LETTURA
             * 48: DEVICE INTERVAL TIMER
             * 49: DEVICE PLT
             * 
            */
            //ALLOCARE SEMD QUANDO RICHIESTO POI USARE INSERTBLOCKED()
            // se non trovo il corrispettivo semaforo nell'array DEVICES[] lo creo

            int intlNo = current_proc->p_s.gpr[4];
            int dnum = current_proc->p_s.gpr[5];
            int dev_pos = (intlNo - 3)*8 + dnum;

            if (current_proc->p_s.gpr[6] == 1){
                //terminale in lettura
                dev_pos = dev_pos + 8;
            }

            if  (device[dev_pos] == NULL){
                //non c'è ancora nessun semaforo per quel device
                if (check_dev_installation(intlNo, dnum) == 1){
                    //se il device è installato lo creo
                    semd_PTR newsem = allocSemd();
                    device[dev_pos] = newsem; //lo salvo nell'array di semafori dei device
                }else{  return NULL;  }//device non installato --> errore!
            }
            
            //faccio P operation sul semaforo
            SYSCALL(PASSEREN, device[dev_pos]->s_semAdd, 0, 0);

        }else if (current_a0 == 6){
            //get CPU time  SYSCALL(GETCPUTIME, 0, 0, 0);
            unsigned int tmp = STCK(tmp);
            current_proc->p_s.gpr[1] = current_proc->p_time + tmp;
            //INCREMENTIAMO IL PC 
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc  + 4; 
            

        }else if (current_a0 == 7){
            //wait for clock    SYSCALL(WAITCLOCK, 0, 0, 0);
  
            if  (device[48] == NULL){
                //non c'è ancora nessun semaforo per quel device
                semd_PTR newsem = allocSemd();
                device[48] = newsem;
            }
            
            //faccio P operation sul semaforo
            SYSCALL(PASSEREN, device[48]->s_semAdd, 0, 0);


        }else if (current_a0 == 8){
            //get support data     p_support = SYSCALL(GETSUPPORTPTR, 0, 0, 0);
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc  + 4; 
            return current_proc->p_supportStruct;
        }  
    }
    
}

void trap_handler(){


}

void interrupt_handler(){
    /**
    INT C=0 , i;
    
    WHILE (C == 0 && i < 8):
        IF VETTORE[i]==1 THEN:
            C = i;
        ENDIF
    ENDFOR

    SWITCH C
        CASE C=0
            "Restituire controllo a current_proc"
        CASE 3<=C<=7
            DEVICE_INTERRUPT_HANDLER();
            interrupt_handler();
        CASE C=1
            PLT_INTERRUPT_HANDLER();
            interrupt_handler();
        CASE C=2
            IT_INTERRUPT_HANDLER();
            interrupt_handler();
    ENDSWITCH


    */
   
}

void init_devices() {
    for (int i = 0; i < 50; i = i + 1) { device[i] = NULL; }
}

int check_dev_installation( int numLine, int numDev){

    unsigned int x, mask; 
    unsigned int* base_line = 0x1000002C;
    if (numLine == 4){ base_line = base_line + 0x04;}
    if (numLine == 5){ base_line = base_line + 0x08;}
    if (numLine == 6){ base_line = base_line + 0x0C;}
    if (numLine == 7){ base_line = base_line + 0X10;}
    x = *base_line;
    mask = power(2, numDev);
    if ( ((x & mask) >> numDev) > 0){ return TRUE; } 
    else { return FALSE; }
}

int check_dev_interruption( int numLine, int numDev){

    unsigned int x, mask; 
    unsigned int* base_line = 0x10000040;
    if (numLine == 4){ base_line = base_line + 0x04;}
    if (numLine == 5){ base_line = base_line + 0x08;}
    if (numLine == 6){ base_line = base_line + 0x0C;}
    if (numLine == 7){ base_line = base_line + 0X10;}
    x = *base_line;
    mask = power(2, numDev);
    if ( ((x & mask) >> numDev) > 0){ return TRUE; } 
    else { return FALSE; }
}


int power(int base, int exp){
    int s=1;
    for (int i=0; i< exp; i = i+1){ s = s*base; }
    return s;
}