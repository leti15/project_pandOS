#include "exceptions_handler.h"

/**   device[] (= array di semafori) è organizzato come segue:
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

void fooBar()
{ //appena entra qui e' in kernel mode con interrupt disabilitati!!!

    state_t *state_reg = (state_t *)BIOSDATAPAGE;
    current_proc->p_s = *state_reg;
    sys4();

    unsigned int current_causeCode = getCAUSE();
    int exCode = CAUSE_GET_EXCCODE(current_causeCode);

    //in base all' exCode capisco cosa fare
    if (exCode == 0)
    {
        interrupt_handler();
    } //External Device Interrupt
    else if (exCode >= 1 && exCode <= 3)
    {
        PassUpOrDie(PGFAULTEXCEPT);
    } //eccezioni TLB
    else if ((exCode > 3 && exCode < 8) || (exCode > 8 && exCode < 13))
    {
        PassUpOrDie(GENERALEXCEPT);
    } //program trap exception handler
    else if (exCode == 8)
    {
        SYS_handler();
    } //system calls
    else
    {
        current_proc->p_supportStruct = NULL;
        PassUpOrDie(GENERALEXCEPT);
    }
}

void SYS_handler()
{
    if (STATUSC_GET_MODE(current_proc->p_s.status) == 1)
    {
        sys_terminate();
    } //il processo era in user mode!
    else
    {
        int current_a0 = current_proc->p_s.reg_a0;
        if (current_a0 == 1) //create process    SYSCALL(CREATEPROCESS, &current_proc->p_s, current_proc->p_supportStruct, 0);
        {
            sys1();
            if (current_proc->p_s.reg_a1 == NULL)
            {
                sys_terminate();
            }

            pcb_PTR newProc = allocPcb(); //creo la stuttura per il nuovo processo

            if (newProc == NULL)
            {
                current_proc->p_s.gpr[1] = -1;
            } //non ci sono pcb liberi= mancanza risorse
            //setto campi del nuovo pcb
            insertChild(current_proc, newProc);
            newProc->p_time = 0;
            newProc->p_supportStruct = (support_t *)current_proc->p_s.gpr[5];
            state_t *temp = (state_t *)current_proc->p_s.gpr[4];
            newProc->p_s = *temp;

            insertProcQ(&readyQ, newProc); //loinserisco nella readyQ

            current_proc->p_s.reg_v0 = 0;

            //INCREMENTIAMO IL PC
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc + 4;

            //incremento il contatore dei processi attivi
            proc_count = proc_count + 1;
        }
        else if (current_a0 == 2) //terminate process SYSCALL(TERMPROCESS, 0, 0, 0);
        {
            sys2();
            sys_terminate();
        }
        else if (current_a0 == 3) //passeren            SYSCALL(PASSEREN, current_proc->p_semAdd, 0, 0);
        {
            sys3();
            sys_p(current_proc->p_s.reg_a1);
        }
        else if (current_a0 == 4) //verhogen  SYSCALL(VERHOGEN, current_proc->p_semAdd, 0, 0);
        {
            sys4();
            sys_v(current_proc->p_s.reg_a1);

            //INCREMENTIAMO IL PC
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc + 4;

            LDST((state_t *)&current_proc->p_s);
        }
        else if (current_a0 == 5) //wait for IO    SYSCALL(IOWAIT, intlNo, dnum, termRead);
        {
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
            sys5();
            if (current_proc->p_s.reg_a1 == NULL || current_proc->p_s.reg_a2 == NULL)
            {
                sys_terminate();
            }

            int intlNo = current_proc->p_s.reg_a1, dnum = current_proc->p_s.reg_a2;
            int dev_pos = (intlNo - 3) * 8 + dnum;

            if (current_proc->p_s.reg_a3 == 1)
            {
                dev_pos = dev_pos + 8;
            } //è un terminale in lettura = READ

            //se non trovo il semforo corrispettivo al device nell'array (ovvero in 'device[dev_pos]') lo creo
            if (device[dev_pos] == NULL)
            {
                //se il device è correttamente stato installato procedo
                if (check_dev_installation(intlNo, dnum) == 1)
                {
                    //prima di inserirlo nella coda del semaforo aggiorno il current process
                    //INCREMENTIAMO IL PC
                    current_proc->p_s.pc_epc = current_proc->p_s.pc_epc + 4;

                    //INCREMENTIAMO IL CPU TIME
                    int tmp = STCK(tmp);
                    current_proc->p_time = current_proc->p_time + tmp;

                    //incrementiamo soft blocked
                    softB_count = softB_count + 1;

                    //faccio una specie di P operation
                    //uso insert blocked per mettere in coda il pcb al nuovo semaforo con descrittore -1 (creato nella insertBlocked)
                    semd_PTR newsem = insertBlocked(&resource_dev, current_proc);
                    if (newsem == TRUE)
                    {
                        PANIC();
                    } // se la insert blocked restituisce TRUE qualcosa è andato storto
                    else
                    {
                        device[dev_pos] = newsem;
                    } //aggiorno array di semafori dei device

                    scheduler();
                }
            }
            else
            {
                //faccio P operation sul semaforo (già esistente)
                sys_p(device[dev_pos]->s_semAdd);
            }
        }
        else if (current_a0 == 6) //get CPU time  SYSCALL(GETCPUTIME, 0, 0, 0);
        {
            sys6();

            //AGGIORNIAMO CPU TIME
            unsigned int tmp = STCK(tmp);
            current_proc->p_s.gpr[1] = current_proc->p_time + tmp;

            //INCREMENTIAMO IL PC
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc + 4;

            LDST((state_t *)&current_proc->p_s);
        }
        else if (current_a0 == 7) //wait for clock    SYSCALL(WAITCLOCK, 0, 0, 0);
        {
            sys7();

            //INCREMENTIAMO IL PC
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc + 4;

            //INCREMENTIAMO IL CPU TIME
            int tmp = STCK(tmp);
            current_proc->p_time = current_proc->p_time + tmp;

            //incrementiamo softblocked se è un semAdd di device
            softB_count = softB_count + 1;

            if (device[48] == NULL)
            {
                //non c'è ancora nessun semaforo per quel device
                semd_PTR newsem = insertBlocked(&resource_dev, current_proc);
                if (newsem == TRUE)
                {
                    PANIC();
                }
                else
                {
                    device[48] = newsem;
                }
            }
            else
            {

                *device[48]->s_semAdd = *device[48]->s_semAdd - 1;
                if (insertBlocked(device[48]->s_semAdd, current_proc) == TRUE)
                {
                    PANIC();
                }
            }

            scheduler();
        }
        else if (current_a0 == 8) //get support data     p_support = SYSCALL(GETSUPPORTPTR, 0, 0, 0);
        {
            sys8();

            //INCREMENTIAMO IL PC
            current_proc->p_s.pc_epc = current_proc->p_s.pc_epc + 4;

            current_proc->p_s.gpr[1] = (unsigned int)current_proc->p_supportStruct;
            LDST((state_t *)&current_proc->p_s);
        }
        else
        {
            //wrong sys call number (reg_a0 >= 9)
            PassUpOrDie(GENERALEXCEPT);
        }
    }
}

int interrupt_handler()
{
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

    unsigned int current_causeCode = getCAUSE();
    int numLine = -1;
    unsigned int IP = 0, mask = 0b00000000000000001111111100000000;
    IP = MASKySHIFTz(current_causeCode, mask, 8);
    //qui IP = 0b0000000000000000xxxxxxx (andiamo ad analizzare gli x)

    if (IP % 2 != 0)
    {
        PANIC();
    }
    else
    {
        IP = IP >> 1;
        if (IP == 0)
        {
            return TRUE;
        } //non ci sono più interrupt pendenti, si può uscire dall'handler

        if (IP % 2 != 0) //PLT interrupt handler
        {
            sysPLT();
            //è acceso l'ultmo bit quindi c'è un interrupt pendente sulla linea  1
            numLine = 1;

            //carica 5 millisecondi in PLT
            unsigned int tmp = STCK(tmp);
            current_proc->p_time = current_proc->p_time + tmp;
            setTIMER(5000 * (*((cpu_t *)TIMESCALEADDR)));

            //salvo lo stato
            state_t *state_register = (state_t *)BIOSDATAPAGE;
            current_proc->p_s = *state_register;

            //inserisco il pcb nella readyQ
            insertProcQ(&readyQ, current_proc);

            //passo il controllo allo scheduler
            scheduler();
        }

        IP = IP >> 1;
        if (IP % 2 != 0) //INTERVAL TIMER interrupt handler
        {
            sysIT();
            //è acceso l'ultmo bit quindi c'è un interrupt pendente sulla linea  2
            numLine = 2;
            int *temp;
            LDIT(100000); // setto interval timer di 100 millisecondi (=100000 microsecondi)

            if (device[48] == NULL)
            {
                //INCREMENTIAMO IL CPU TIME
                int tmp = STCK(tmp);
                current_proc->p_time = current_proc->p_time + tmp;

                //incrementiamo softblocked se è un semAdd di device
                softB_count = softB_count + 1;

                //non c'è ancora nessun semaforo per quel device quindi faccio una P
                semd_PTR newsem = insertBlocked((int *)&resource_it, current_proc);
                if (newsem == TRUE)
                {
                    PANIC();
                }
                else
                {
                    device[48] = newsem;
                }
            }

            while (device[48] != NULL && emptyProcQ(device[48]->s_procQ) != TRUE)
            {
                //finchè non ho svuotato la coda dei processi bloccati all' interval timer
                //aggiungo una risorsa al semaforo corrispondente

                *(device[48]->s_semAdd) = *(device[48]->s_semAdd) + 1;

                if (*(device[48]->s_semAdd) - 1 < 0)
                {
                    //se ora c'è una risorsa disponibile la uso, quindi riesumo un pcb e lo metto nella readyQ
                    temp = device[48]->s_semAdd;
                    pcb_PTR newpcb = removeBlocked(device[48]->s_semAdd);
                    insertProcQ(&readyQ, newpcb);
                    //diminuisco soft blocked
                    softB_count = softB_count - 1;
                    if (headBlocked(temp) == NULL)
                    {
                        device[48] = NULL;
                    }
                }
            }

            /*
            serve ripristinare le risorse?????
            se non abbiamo un semaforo fisso per l'interval timer non penso serva onestamente...
                    if(*(device[48]->s_semAdd) == 0){ sys1();}
                    else{
                        *(device[48]->s_semAdd) = 0;
                        sys2();
                    }
            */

            LDST((state_t *)BIOSDATAPAGE);
        }

        IP = IP >> 1;
        numLine = 3;
        while (IP != 0 && numLine <= 7) //DEVICE INTERRUPT handler
        {
            sysDEV();
            //ci sono device interrupts
            while (IP % 2 == 0)
            {
                //finchè non sono in corrispondenza del bit della linea con pending interrupt faccio lo shift
                IP = IP >> 1;
                numLine = numLine + 1;
            }

            /*  arrivati qui avro' interrupt pendenti sulla linea 'numLine' perchè il
            *  (numLine)-esimo bit sarà acceso */

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
            while (check_dev_interruption(numLine, numDev) == FALSE)
            {
                numDev = numDev + 1;
            }

            unsigned int mask, device_status, base_device_reg;
            //troviamo la base del device register
            devreg_t *reg = GET_devAddrBase(numLine, numDev);
            int position = (numLine - 3) * 8 + numDev;

            if (numLine == 7)
            { //noi facciamo operazione write --> device fa operazione recive =0x0
                //noi facciamo operazione read --> device fa operazione trasmit =0x8

                if ((reg->term.recv_status) != READY)
                {
                    //troviamo lo status del device
                    device_status = reg->term.recv_status;
                    position = position + 8;
                    sys6();
                    if (device[position] != NULL)
                    { //faccio operazione V

                        pcb_PTR unblocked = sys_v(device[position]->s_semAdd);
                        if (unblocked != NULL)
                        {
                            outProcQ(&readyQ, unblocked);
                            unblocked->p_s.reg_v0 = device_status;
                            //interrupt su subdevice RECIVE perchè != da ready (dopo una nostra richiesta di write)
                            //setto campo COMMAND del device register recive con ACK
                            reg->term.recv_command = ACK;
                            insertProcQ(&readyQ, unblocked);
                        }
                    }
                }

                if ((reg->term.transm_status) != READY)
                {
                    //position = position + 8;
                    sys7();
                    if (device[position] != NULL)
                    { //faccio operazione V
                        pcb_PTR unblocked = sys_v(device[position]->s_semAdd);
                        if (unblocked != NULL)
                        {
                            outProcQ(&readyQ, unblocked);
                            device_status = reg->term.transm_status;
                            unblocked->p_s.reg_v0 = device_status;
                            //interrupt su subdevice TRANSMIT perchè != ready (dopo nostra operazione read)
                            //setto campo COMMAND del device register trasmit con ACK
                            reg->term.transm_command = (reg->term.transm_command & 0xFFFFFF00) + ACK;
                            //troviamo lo status del device

                            //incremento position per accedere ai semafori in read
                            insertProcQ(&readyQ, unblocked);
                        }
                    }
                }
            }
            else
            { //se non ho un terminale (linea 7)

                reg->dtp.command = ACK;
                device_status = reg->dtp.status;

                if (device[position] != NULL)
                {
                    ///faccio operazione V
                    pcb_PTR unblocked = sys_v(device[position]->s_semAdd);

                    if (unblocked != NULL)
                    {
                        outProcQ(&readyQ, unblocked);
                        unblocked->p_s.reg_v0 = device_status;
                        insertProcQ(&readyQ, unblocked);
                    }
                }
            }

            //return control to current process
            LDST((state_t *)BIOSDATAPAGE);
        }
    }
}

void sys_terminate()
{
    breakPoint2();
    pcb_PTR tempQueue = mkEmptyProcQ(); //frangia dei pcb non ancora terminati
    pcb_PTR temp = current_proc;
    outChild(current_proc);
    insertProcQ(&tempQueue, current_proc);
    //emptyChild == 1 se NON ha figli

    while (emptyProcQ(tempQueue) != 1) //finchè non è vuota continuo
    {
        pcb_PTR to_terminate = removeProcQ(&tempQueue);

        while (emptyChild(to_terminate) != 1)
        { //se ha figli
            pcb_PTR child_toRemove = removeChild(to_terminate);
            insertProcQ(&tempQueue, child_toRemove);
        }

        int bool_dev_sem;
        //a questo punto 'to_terminate' non avrà più figli e lo posso terminare
        if (to_terminate->p_semAdd != NULL)
        {
            bool_dev_sem = check_dev_semAdd(to_terminate->p_semAdd);
            if (!bool_dev_sem && outBlocked(to_terminate) != NULL)
            {
                //aggiusta semaforo relativo
                *(to_terminate->p_semAdd) = *(to_terminate->p_semAdd) + 1;
            }
        }

        outProcQ(&readyQ, to_terminate);
        freePcb(to_terminate);
        proc_count = proc_count - 1;
        if (bool_dev_sem)
        {
            softB_count = softB_count - 1;
        }
    }

    //chiamimamo lo scheduler
    scheduler();
}

void sys_p(int *temp)
{
    syspasse();
    if (temp == NULL)
    {
        sys_terminate();
    }
    else
    {
        *temp = *temp - 1; //decremento risorse

        current_proc->p_s.pc_epc = current_proc->p_s.pc_epc + 4; //INCREMENTIAMO IL PC

        if (*(temp) < 0 && headBlocked(temp) == NULL)
        {
            //INCREMENTIAMO IL CPU TIME
            int tmp = STCK(tmp);
            current_proc->p_time = current_proc->p_time + tmp;

            //lo stato del pcb (state_t) lo abbiamo già aggiornato all'inizio di fooBar

            if (check_dev_semAdd(temp) == TRUE)
            {
                softB_count = softB_count + 1;
            } //incrementiamo softblocked se è un semAdd di device
            if (insertBlocked(temp, current_proc) == TRUE)
            {
                PANIC();
            } //inserisco il pcb nella coda del semaforo
            scheduler();
        }
        else
        {
            LDST(&(current_proc->p_s));
        }
    }
}

void *sys_v(int *temp)
{
    sysverog();
    if (temp == NULL)
    {
        sys_terminate();
    }
    else
    {
        pcb_PTR newpcb;
        *temp = *temp + 1; //aumento risorse
        if (*temp - 1 < 0)
        { //ora c'è una risorsa disponibile quindi sblocco un processo
            newpcb = removeBlocked(temp);

            if (newpcb != NULL) //se è andato tutto bene (removeBlocked ha restituito un pcb)
            {
                newpcb->p_semAdd = NULL;      //pcb non più bloccato
                insertProcQ(&readyQ, newpcb); //lo inserisco nella readyQ

                if (check_dev_semAdd(temp) == TRUE) //se è un semaforo di un DEVICE
                {
                    /*  se è NULL significa che il semaforo 'temp' non è più attivo perchè abbiamo tolto 
                     *  l'ultimo pcb bloccato e quindi il suo relativo campo in device[] va settato a NULL    */
                    if (headBlocked(temp) == NULL)
                    {
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

void PassUpOrDie(int EXCEPT)
{
    breakPoint3();

    if (current_proc->p_supportStruct != NULL)
    {
        current_proc->p_supportStruct->sup_exceptState[EXCEPT] = *(state_t *)BIOSDATAPAGE;
        LDCXT(current_proc->p_s.gpr[26], current_proc->p_s.status, current_proc->p_s.pc_epc);
    }
    else
    {
        sys_terminate();
    }
}