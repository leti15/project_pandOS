#include "commons.h"
#include "exceptions_handler.h"

/** TODO: 
 *  --sistemare terminae process leggendo pg. 38 del pdf (cap 3.9)
            ->sistemare anche il soft blocked
 *  --implementa "PassUpOrDie"
 *  --sistemare V
*/

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
    current_proc->p_s = *state_reg;

    unsigned int current_causeCode = getCAUSE();
    int exCode = CAUSE_GET_EXCCODE(current_causeCode); 

    //in base all' exCode capisco cosa fare
    if (exCode == 0){
        //External Device Interrupt
        interrupt_handler(current_causeCode, exCode);
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
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc + 4; 

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



                /*  controllare se era un pcb bloccato ad un semaforo di un device, 
                 *  in quel caso va decrementato anche 'softBlocked'  */
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
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc + 4; 

            if (*temp < 0) {
                //INCREMENTIAMO IL CPU TIME 
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
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc + 4; 

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

            //va fatto altro.................................!!!!!!!!!


        }else if (current_a0 == 6){
            //get CPU time  SYSCALL(GETCPUTIME, 0, 0, 0);

            //AGGIORNIAMO CPUT TIME
            unsigned int tmp = STCK(tmp);
            current_proc->p_s.gpr[1] = current_proc->p_time + tmp;

            //INCREMENTIAMO IL PC 
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc + 4; 
    
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

            //INCREMENTIAMO IL PC
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc  + 4; 

            current_proc->p_s.gpr[1] = current_proc->p_supportStruct;
        }else{
            //wrong sys call number (reg_a0 >= 9)
            /**dobbiamo eseguire un'operazione "pass up or die" 
             * usando il valore dell'indice ' GENERALEXCEPT '
             */
        }
    }
    
}

void trap_handler(){


}

void interrupt_handler(unsigned int current_causeCode, int exCode){
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
   
    int numLine = -1;
    unsigned int IP = 0, mask = 0b00000000000000001111111100000000;
    IP = MASKySHIFTz(current_causeCode, mask, 9);
    //qui IP = 0b0000000000000000xxxxxxx (andiamo ad analizzare gli x)

    if(IP == 0){ return; } //non ci sono più interrupt pendenti, si può uscire dall'handler

    if (IP % 2 == 0){
        //è acceso l'ultmo bit quindi c'è un interrupt pendente sulla linea  1
        //PLT interrupt handler
        numLine = 1;
    }

    IP = IP >> 1;
    if (IP % 2 == 0){
        //è acceso l'ultmo bit quindi c'è un interrupt pendente sulla linea  2
        //INTERVAL TIMER interrupt handler
        numLine = 2;
    }

    IP = IP >> 1;
    if (IP != 0) {
        //ci sono device interrupts
        for (int i = 3; i<8; i= i+1){
            if (IP%2 != 0){
                numLine = i;
                break; //appena trova una lina con interrupt esce
            }
            IP = IP >> 1;
        }

        //gestione device interrupt 
        /** 1. calculate device egster
         *  2. save 'status code' from the device register (salviamo l'intero registro?)
         *  3. write che 'command code' on the register (di ack o no)
         *  4. perform V operation, unblock che process
         *  5. place the saved 'status code' on the unblocked pcb's reg_v0
         *  6. put che unblocked pcb un the ready queue
         *  7. return control to the current process, perform a LDST on the 
         *     saved exception state at BIOSDATAPAGE
        */

    }

/*
    if (IP % 2 == 0){
        //è acceso l'ultmo bit quindi c'è un interrupt pendente sulla linea  3
        //DISK DEVICES interrupt handler
    }

    IP = IP >> 1;
    if (IP % 2 == 0){
        //è acceso l'ultmo bit quindi c'è un interrupt pendente sulla linea  4
        //FLASH DEVICES interrupt handler
    }

    IP = IP >> 1;
    if (IP % 2 == 0){
        //è acceso l'ultmo bit quindi c'è un interrupt pendente sulla linea  5
        //NETWORK DEVICES interrupt handler
    }

    IP = IP >> 1;
    if (IP % 2 == 0){
        //è acceso l'ultmo bit quindi c'è un interrupt pendente sulla linea  6
        //PRINTER DEVICES interrupt handler
    }

    IP = IP >> 1;
    if (IP % 2 == 0){
        //è acceso l'ultmo bit quindi c'è un interrupt pendente sulla linea  7
        //TERMINAL DEVICES interrupt handler
    }
*/


}

