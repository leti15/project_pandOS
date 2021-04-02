
#include "exceptions_handler.h"

  /**
             * 0-7: DEVICE LINEA 3
             * 8-15:DEVICE LINEA 4
             * 16-23: DEVICE LINEA 5
             * 24-31: DEVICE LINEA 6
             * 32-47: DEVICE LINEA 7 (TERMINALI)
             *      32-39: TERMINALI IN SCRITTURA
             *      40-47: TERMINALI IN LETTURA
             * 48: DEVICE INTERVAL TIMER
             * 
            */


void fooBar(){
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
    state_t* state_reg = (state_t*) BIOSDATAPAGE;
    current_proc->p_s = *state_reg;

    unsigned int current_causeCode = getCAUSE();
    int exCode = CAUSE_GET_EXCCODE(current_causeCode); 
    
    //in base all' exCode capisco cosa fare
    if (exCode == 0){
        //External Device Interrupt
        interrupt_handler();
    }else if(exCode >=1 && exCode <= 3){
        //eccezioni TLB
        PassUpOrDie(PGFAULTEXCEPT);
    }else if( (exCode > 3 && exCode <= 7) || (exCode > 8 && exCode <= 12) ){
        //program trap exception handler
        PassUpOrDie(GENERALEXCEPT);
    }else if(exCode == 8){
        //system calls
        SYS_handler();
    }else{
        current_proc->p_supportStruct = ((void *)0xFFFFFFFF);
        PassUpOrDie(GENERALEXCEPT);
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
         sys_terminate();
    }else{
        int current_a0 = current_proc->p_s.reg_a0;
        if (current_a0 == 1){
            sys1();
            //create process    SYSCALL(CREATEPROCESS, &current_proc->p_s, current_proc->p_supportStruct, 0);
            if (current_proc->p_s.gpr[4] == ((void *)0xFFFFFFFF)){  sys_terminate(); } 
            
            pcb_PTR newProc = allocPcb();
            if (newProc == NULL){ 
                //non ci sono pcb liberi= mancanza risorse
                current_proc->p_s.gpr[1] = -1;
            }
            proc_count = proc_count + 1;
            insertChild(current_proc, newProc);
            newProc->p_time = 0;
            newProc->p_supportStruct = (support_t*) current_proc->p_s.gpr[5];
            state_t* temp = (state_t*) current_proc->p_s.gpr[4];
            newProc->p_s = *temp;
            insertProcQ(&readyQ, newProc);
            current_proc->p_s.gpr[1] = 0;

            //INCREMENTIAMO IL PC 
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc + 4; 

       }else if (current_a0 == 2){
            //terminate process SYSCALL(TERMPROCESS, 0, 0, 0);
            sys2();
            sys_terminate();

        }else if (current_a0 == 3){
            sys3();
            //passeren            SYSCALL(PASSEREN, current_proc->p_semAdd, 0, 0);
            sys_p(current_proc->p_s.reg_a1);

        }else if (current_a0 == 4){
            sys4();
            //verhogen  SYSCALL(VERHOGEN, current_proc->p_semAdd, 0, 0);
            sys_v(current_proc->p_s.reg_a1);

            //INCREMENTIAMO IL PC 
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc + 4;
            
            LDST((state_t*) &current_proc->p_s);

        }else if (current_a0 == 5){
            sys5();
            //wait for IO    SYSCALL(IOWAIT, intlNo, dnum, termRead);
            if (current_proc->p_s.reg_a1 == NULL || current_proc->p_s.gpr[5] == NULL )
            {  sys_terminate();  }
            
            /**
             * 0-7: DEVICE LINEA 3
             * 8-15:DEVICE LINEA 4
             * 16-23: DEVICE LINEA 5
             * 24-31: DEVICE LINEA 6
             * 32-47: DEVICE LINEA 7 (TERMINALI)
             *      32-39: TERMINALI IN SCRITTURA=WRITE
             *      40-47: TERMINALI IN LETTURA=READ
             * 48: DEVICE INTERVAL TIMER
             * 
            */
            //ALLOCARE SEMD QUANDO RICHIESTO POI USARE INSERTBLOCKED()
            // se non trovo il corrispettivo semaforo nell'array DEVICES[] lo creo

            int intlNo = current_proc->p_s.gpr[4];
            int dnum = current_proc->p_s.gpr[5];
            int dev_pos = (intlNo - 3)*8 + dnum;
 
            if (current_proc->p_s.reg_a3 == 1){
                //terminale in lettura
                dev_pos = dev_pos + 8;
            }

            if (device[dev_pos] == NULL ){
                
                //non c'è ancora nessun semaforo per quel device
                if (check_dev_installation(intlNo, dnum) == 1){

                    //aggiorno il current process
                    //INCREMENTIAMO IL PC 
                    current_proc->p_s.pc_epc = current_proc->p_s.pc_epc + 4; 

                    //INCREMENTIAMO IL CPU TIME 
                    int tmp = STCK(tmp);
                    current_proc->p_time = current_proc->p_time + tmp;

                    //incremento soft blocked
                    softB_count = softB_count + 1;

                    //se il device è installato lo creo e inserisco già il processo bloccato (fare una P)
                   
                    semd_PTR newsem = insertBlocked( &resource_dev, current_proc);
                    if( newsem == TRUE){ PANIC();}
                    else{ device[dev_pos] = newsem; } //lo salvo nell'array di semafori dei device
                       
                    scheduler();
                }      
            }else{
                //faccio P operation sul semaforo
                sys_p(device[dev_pos]->s_semAdd);
            }

        }else if (current_a0 == 6){
            sys6();
            //get CPU time  SYSCALL(GETCPUTIME, 0, 0, 0);

            //AGGIORNIAMO CPU TIME
            unsigned int tmp = STCK(tmp);
            current_proc->p_s.gpr[1] = current_proc->p_time + tmp;

            //INCREMENTIAMO IL PC 
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc + 4; 
            LDST((state_t*) &current_proc->p_s);
    
        }else if (current_a0 == 7){
            sys7();
            //wait for clock    SYSCALL(WAITCLOCK, 0, 0, 0);

            //INCREMENTIAMO IL PC 
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc + 4; 
    
            //INCREMENTIAMO IL CPU TIME 
            int tmp = STCK(tmp);
            current_proc->p_time = current_proc->p_time + tmp;

            //incrementiamo softblocked se è un semAdd di device
            softB_count = softB_count + 1;
  
            if  (device[48] == NULL){
                //non c'è ancora nessun semaforo per quel device
                semd_PTR newsem = insertBlocked(&resource_dev, current_proc);
                if(newsem == TRUE){ PANIC();}
                else{ device[48] = newsem; }
            }else{

                *device[48]->s_semAdd = *device[48]->s_semAdd -1; 
                if(insertBlocked(device[48]->s_semAdd, current_proc) == TRUE){ PANIC();}
            }
            
            scheduler();
        
        }else if (current_a0 == 8){
            sys8();
            //get support data     p_support = SYSCALL(GETSUPPORTPTR, 0, 0, 0);

            //INCREMENTIAMO IL PC
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc  + 4; 

            current_proc->p_s.gpr[1] = (unsigned int) current_proc->p_supportStruct;
            LDST((state_t*) &current_proc->p_s);
        }else{
            //wrong sys call number (reg_a0 >= 9)
            PassUpOrDie(GENERALEXCEPT);
        }
    } 
}


int interrupt_handler(){
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
   breakPoint();

    unsigned int current_causeCode = getCAUSE();
    int numLine = -1;
    unsigned int IP = 0, mask = 0b00000000000000001111111100000000;
    IP = MASKySHIFTz(current_causeCode, mask, 8);
    //qui IP = 0b0000000000000000xxxxxxx (andiamo ad analizzare gli x)

    if (IP % 2 != 0){ PANIC();}
    else{
        IP = IP >> 1;
        if(IP == 0){ return TRUE; } //non ci sono più interrupt pendenti, si può uscire dall'handler

        if (IP % 2 != 0){
            sysPLT();
            //è acceso l'ultmo bit quindi c'è un interrupt pendente sulla linea  1
            //PLT interrupt handler
            numLine = 1;

            //carica 5 millisecondi in PLT
            unsigned int tmp = STCK(tmp);
            current_proc->p_s.gpr[1] = current_proc->p_time + tmp;
            setTIMER(5000 *(*((cpu_t *) TIMESCALEADDR)));

            //salvo lo stato 
            state_t* state_register = (state_t*) BIOSDATAPAGE;
            current_proc->p_s = *state_register;

            //inserisco il pcb nella redayQ
            insertProcQ(&readyQ, current_proc);

            //passo il controllo allo scheduler
            scheduler();
        }

        IP = IP >> 1;
        if (IP % 2 != 0){
            sysIT();
            //è acceso l'ultmo bit quindi c'è un interrupt pendente sulla linea  2
            //INTERVAL TIMER interrupt handler
            numLine = 2;
            LDIT(100000);  // setto interval timer di 100 millisecondi (=100000 microsecondi)

            if(device[48] == NULL){
                //INCREMENTIAMO IL CPU TIME 
                int tmp = STCK(tmp);
                current_proc->p_time = current_proc->p_time + tmp;

                //incrementiamo softblocked se è un semAdd di device
                softB_count = softB_count + 1;
                
                //non c'è ancora nessun semaforo per quel device quindi faccio una P
                semd_PTR newsem = insertBlocked((int*)&resource_it, current_proc);
                if(newsem == TRUE){ PANIC();}
                else{ device[48] = newsem; }
            }
            
            while ( device[48] != NULL && emptyProcQ(device[48]->s_procQ) != 1){
                //finchè non ho svuotato la coda dei processi bloccati all' interval timer
                //aggiungo una risorsa al semaforo corrispondente
                
                *(device[48]->s_semAdd) = *(device[48]->s_semAdd)  + 1;

                if(*(device[48]->s_semAdd)  - 1 < 0){
                    
                        //se ora c'è una risorsa disponibile la uso, quindi riesumo un pcb e lo metto nella readyQ
                        int* temp = device[48]->s_semAdd;
                        pcb_PTR newpcb = removeBlocked(device[48]->s_semAdd);
                        insertProcQ(&readyQ, newpcb);
                        //diminuisco soft blocked
                        softB_count = softB_count - 1; 
                        if(headBlocked(temp) == NULL){ device[48] = NULL; }
                        
                }
            }
            sys2();

            /*
            serve ripristinare le risorse?????
            se non abbiamo un semaforo fisso per l'interval timer non penso servi onestamente...



            //ripristino le risorse del semaforo
            if(*(device[48]->s_semAdd) == 0){ return TRUE;}
            else{
                    *(device[48]->s_semAdd) = 0;
                    return FALSE;
            }
            */


            LDST((state_t*) BIOSDATAPAGE);
        }

        IP = IP >> 1;
        numLine = 3;
        while (IP != 0 && numLine <= 7) {
            sysDEV();
                    //ci sono device interrupts
                     while(IP % 2 == 0){
                        //finchè non sono in corrispondenza del bit della linea con pending interrupt faccio lo shift 
                        IP = IP >> 1;
                        numLine = numLine + 1;
                    }

                    /*  arrivati qui avro' interrupt pendenti sulla linea 'numLine' perchè il
                    *  (numLine)-esimo bit sarà acceso */

                    //gestisco device interrupt 
                    /**1. calculate device register
                    *  2. save 'status code' from the device register (salviamo l'intero registro?)
                    *  3. write che 'command code' on the register (di ack o no)
                    *  4. perform V operation, unblock che process
                    *  5. place the saved 'status code' on the unblocked pcb's reg_v0
                    *  6. put che unblocked pcb un the ready queue
                    *  7. return control to the current process, perform a LDST on the 
                    *     saved exception state at BIOSDATAPAGE
                    */
                
                    //identifico quale device è interrotto attraverso Interruption Bit Map
                    int numDev = 0;
                    while (check_dev_interruption(numLine, numDev) == FALSE){ numDev = numDev + 1;}
                    
                    unsigned int mask, device_status, base_device_reg;
                    //troviamo la base del device register
                    devreg_t* reg = GET_devAddrBase(numLine, numDev);
                    int position = (numLine - 3)*8 + numDev;

                    if (numLine == 7){
                        //noi facciamo operazione write --> device fa operazione recive =0x0 
                        //noi facciamo operazione read --> device fa operazione trasmit =0x8
                        if (reg->term.recv_status != READY){
                            //interrupt su subdevice RECIVE perchè != da ready (dopo una nostra richiesta di write)
                            //setto campo COMMAND del device register recive con ACK
                            reg->term.recv_command = ACK;
                            //troviamo lo status del device
                            device_status = reg->term.recv_status;

                            if(device[position] != NULL){
                                //faccio operazione V
                                pcb_PTR unblocked = sys_v(device[position]->s_semAdd);
                                if(unblocked != NULL){ 
                                    outProcQ(&readyQ, unblocked);
                                    unblocked->p_s.reg_v0 = device_status; 
                                    insertProcQ(&readyQ,unblocked);
                                }
                            }
                        }
                        if( reg->term.transm_status != READY){
                            //interrupt su subdevice TRANSMIT perchè != ready (dopo nostra operazione read)
                            //setto campo COMMAND del device register trasmit con ACK
                            reg->term.transm_command = (reg->term.transm_command & 0xFFFFFF00) + ACK;
                            //troviamo lo status del device
                            device_status = reg->term.transm_status;
                            //incremento position per accedere ai semafori in read
                            position = position + 8;
                            
                            if(device[position]!= NULL){
                                //faccio operazione V
                                pcb_PTR unblocked = sys_v(device[position]->s_semAdd);
                                if(unblocked != NULL){ 
                                    outProcQ(&readyQ, unblocked);
                                    unblocked->p_s.reg_v0 = device_status; 
                                    insertProcQ(&readyQ,unblocked);
                                }
                            }
                        }
                    }else{
                        //se non ho un terminale (linea 7)
                        reg->dtp.command = ACK;
                        device_status = reg->dtp.status;

                        if(device[position]!= NULL){
                                ///faccio operazione V
                                pcb_PTR unblocked = sys_v(device[position]->s_semAdd);
                                
                                if(unblocked != NULL){ 
                                    outProcQ(&readyQ, unblocked);
                                    unblocked->p_s.reg_v0 = device_status; 
                                    insertProcQ(&readyQ,unblocked);
                                }
                        }
                    }
                    
                    //return control to current process
                    LDST((state_t*) BIOSDATAPAGE);
        }
    }
}

void sys_terminate (){
    breakPoint2();
            pcb_PTR tempQueue = mkEmptyProcQ(); //frangia dei pcb non ancora terminati 
            pcb_PTR temp = current_proc;
            outChild(current_proc);
            insertProcQ(&tempQueue, current_proc);
            //emptyChild == 1 se NON ha figli

            while (emptyProcQ(tempQueue) != 1) //finchè non è vuota continuo 
            {
                pcb_PTR to_terminate = removeProcQ(&tempQueue);

                while(emptyChild(to_terminate) != 1)
                {   //se ha figli
                    pcb_PTR child_toRemove = removeChild(to_terminate);
                    insertProcQ(&tempQueue, child_toRemove);
                }

                int bool_dev_sem;
                //a questo punto 'to_terminate' non avrà più figli e lo posso terminare
                if (to_terminate->p_semAdd != NULL){ 
                    bool_dev_sem = check_dev_semAdd(to_terminate->p_semAdd);
                    if (!bool_dev_sem && outBlocked(to_terminate) != NULL){   
                        //aggiusta semaforo relativo
                        *(to_terminate->p_semAdd) = *(to_terminate->p_semAdd) + 1;
                    }
                }
                outProcQ(&readyQ, to_terminate);
                freePcb(to_terminate);
                proc_count = proc_count - 1;
                if(bool_dev_sem){ softB_count = softB_count - 1;}
                
            }
            //chiamimamo lo scheduler
            scheduler();

}

void sys_p (int* temp){
    syspasse();
    if (temp == NULL ){  sys_terminate(); }
    else{

        *temp = *temp - 1;
 breakPoint();
        //INCREMENTIAMO IL PC 
        current_proc->p_s.pc_epc = current_proc->p_s.pc_epc + 4; 
        
        
        if ( *(temp) < 0 && headBlocked(temp) == NULL){
                /** se ora le risorse sono -1, ovvero sono finite, devo bloccare il processo al semaforo quindi:
                 *  -->aggiorno il CPUtime del processo
                 *  -->lo inserisco nella coda del semaforo
                */
            breakPoint2();
            //INCREMENTIAMO IL CPU TIME 
            int tmp = STCK(tmp);
            current_proc->p_time = current_proc->p_time + tmp;

            //lo stato del pcb (state_t) lo abbiamo già aggiornato all'inizio di fooBar

            //incrementiamo softblocked se è un semAdd di device
            if(check_dev_semAdd(temp) == TRUE){ softB_count = softB_count + 1;}
            if(insertBlocked(temp, current_proc) == TRUE){ PANIC(); }
                
            scheduler();
            
        }else{
            breakPoint3();

            LDST(&(current_proc->p_s));
        } 
               
    }

}

void* sys_v (int* temp){
    sysverog();
    
    if (temp == NULL){ sys_terminate(); }
    else {
        pcb_PTR newpcb;
        *temp = *temp + 1; 
        if(*temp -1 < 0){    
            //ora c'è una risorsa disponibile
            newpcb = removeBlocked(temp);
            
            if (newpcb != NULL){ 
                //se è andato tutto bene, quindi removeBlocked ha restituito un pcb, lo inserisco nella readyQ
                newpcb->p_semAdd = NULL;
                insertProcQ(&readyQ, newpcb); 
                
                if(check_dev_semAdd(temp) == TRUE){
                    //se è un semaforo device
                    if (headBlocked(temp) == NULL){
                        /*  se è NULL significa che il semaforo 'temp' non è più attivo perchè abbiamo tolto 
                            l'ultimo pcb bloccato e quindi il suo relativo campo in device[] va settato a NULL    */
                        device[find_dev_index(temp)] = NULL;
                    }
                    //siccome il pcb era bloccato ad un semaforo dei device devo diminuire soft blocked
                    softB_count = softB_count - 1;
                }
            }
        }
        return newpcb;
    }

}

void PassUpOrDie(int EXCEPT){
    breakPoint3();
    if (current_proc->p_supportStruct != NULL){
        current_proc->p_supportStruct->sup_exceptState[EXCEPT] = *(state_t*)BIOSDATAPAGE;
        LDCXT(current_proc->p_s.gpr[26], current_proc->p_s.status, current_proc->p_s.pc_epc);
    }else{
            sys_terminate();
    }
}