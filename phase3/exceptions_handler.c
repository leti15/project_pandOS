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

/*
void uTLB_RefillHandler(){
    //state_reg = (state_t *)BIOSDATAPAGE;
    support_t* support_struct = SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    int owner = (support_struct->sup_exceptState[PGFAULTEXCEPT].entry_hi) & 0b00000000000000000000111111111111;
    int missing_page = (support_struct->sup_exceptState[PGFAULTEXCEPT].entry_hi) >> 12;
    pteEntry_t new_pgEntry = inspecteHI(missing_page, support_struct);

    setENTRYHI( (unsigned int) &new_pgEntry.pte_entryHI);
    setENTRYLO( (unsigned int) &new_pgEntry.pte_entryLO);
    TLBWR();

    LDST(support_struct->sup_exceptState[PGFAULTEXCEPT]);
}
*/
void exception_handler()
{ //appena entra qui e' in kernel mode con interrupt disabilitati!!!

    state_t * state_reg = (state_t *)BIOSDATAPAGE;

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
        sys_terminate();
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
        //INCREMENTIAMO IL PC
        state_t * state_reg = (state_t *)BIOSDATAPAGE;
        state_reg->pc_epc = state_reg->pc_epc + 4;

        int current_a0 = state_reg->reg_a0;
        if (current_a0 == 1) //create process    SYSCALL(CREATEPROCESS, &current_proc->p_s, current_proc->p_supportStruct, 0);
        {
            if (state_reg->reg_a1 == (int)NULL)
            {   sys_terminate();  }

            pcb_PTR newProc = allocPcb(); //creo la stuttura per il nuovo processo
       
            if (newProc == NULL)
            {
                state_reg->reg_v0 = -1;
            } //non ci sono pcb liberi= mancanza risorse
            else
            {
                if (proc_count == 1)
                    current_proc->p_child = NULL;

                //setto campi del nuovo pcb
                insertChild(current_proc, newProc);

                newProc->p_time = 0;
                newProc->p_supportStruct = (support_t *)state_reg->gpr[5];
                state_t *temp = (state_t *)state_reg->gpr[4];
                newProc->p_s = *temp;
                newProc->p_semAdd = NULL;
                insertProcQ(&readyQ, newProc); //loinserisco nella readyQ

                state_reg->reg_v0 = 0;
                proc_count = proc_count + 1; //incremento il contatore dei processi attivi

            }

            LDST(state_reg);
        }
        else if (current_a0 == 2) //terminate process SYSCALL(TERMPROCESS, 0, 0, 0);
        {
            sys_terminate();
        }
        else if (current_a0 == 3) //passeren            SYSCALL(PASSEREN, current_proc->p_semAdd, 0, 0);
        {
            state_t* state_reg = (state_t *)BIOSDATAPAGE;
            sys_p((int *)state_reg->reg_a1);
        }
        else if (current_a0 == 4) //verhogen  SYSCALL(VERHOGEN, current_proc->p_semAdd, 0, 0);
        {
            state_t* state_reg = (state_t *)BIOSDATAPAGE;
            pcb_PTR temp = sys_v((int *)state_reg->reg_a1);
            LDST(state_reg);
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

            state_t* state_reg = (state_t *)BIOSDATAPAGE;
            if (state_reg->reg_a1 == (int)NULL || state_reg->reg_a2 == (int)NULL)
            {
                sys_terminate();
            }

            int intlNo = state_reg->reg_a1, dnum = state_reg->reg_a2;
            int dev_pos = (intlNo - 3) * 8 + dnum;

            if (state_reg->reg_a3 == 1)
            {
                dev_pos = dev_pos + 8;
            } //è un terminale in lettura = READ

            if (check_dev_installation(intlNo, dnum) != 1)
            {
                sys_terminate();
            }

            sys_p(&devicesem[dev_pos]);
        }
        else if (current_a0 == 6) //get CPU time  SYSCALL(GETCPUTIME, 0, 0, 0);
        {

            state_t* state_reg = (state_t *)BIOSDATAPAGE;
            //AGGIORNIAMO CPU TIME
            unsigned int tmp = STCK(tmp);
            state_reg->gpr[1] = current_proc->p_time + tmp - count_time;

            LDST(state_reg);
        }
        else if (current_a0 == 7) //wait for clock    SYSCALL(WAITCLOCK, 0, 0, 0);
        {

            state_t* state_reg = (state_t *)BIOSDATAPAGE;

            //incrementiamo softblocked se è un semAdd di device
            softB_count = softB_count + 1;

            //reset risorse
            devicesem[48] = devicesem[48] - 1;

            //INCREMENTIAMO IL CPU TIME
            int tmp = STCK(tmp);
            current_proc->p_time = current_proc->p_time + tmp - count_time;

            current_proc->p_s = *state_reg;

            if (insertBlocked(&devicesem[48], current_proc) == TRUE)
            {
                PANIC();
            }

            scheduler();
        }
        else if (current_a0 == 8) //get support data     p_support = SYSCALL(GETSUPPORTPTR, 0, 0, 0);
        {
            sys_8();
            /*state_reg->reg_v0 = &(current_proc->p_supportStruct);
            LDST(state_reg);*/
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
            handlerIT(numLine);
        } //non ci sono più interrupt pendenti, si può uscire dall'handler

        if (IP % 2 != 0) //PLT interrupt handler
        {
            //è acceso l'ultmo bit quindi c'è un interrupt pendente sulla linea  1
            numLine = 1;

            //carica 5 millisecondi in PLT
            setTIMER(5000 * (*((cpu_t *)TIMESCALEADDR)));

            //salvo lo stato
            current_proc->p_s = * (state_t *)BIOSDATAPAGE;

            unsigned int tmp = STCK(tmp);
            current_proc->p_time = current_proc->p_time + tmp - count_time;

            //inserisco il pcb nella readyQ
            insertProcQ(&readyQ, current_proc);

            //passo il controllo allo scheduler
            scheduler();
        }

        IP = IP >> 1;
        if (IP % 2 != 0) //INTERVAL TIMER interrupt handler
        {
            handlerIT(numLine);
        }

        IP = IP >> 1;
        numLine = 3;
        while (IP != 0 && numLine <= 7) //DEVICE INTERRUPT handler
        {
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
            devreg_t *reg = (devreg_t *)GET_devAddrBase(numLine, numDev);
            int position = (numLine - 3) * 8 + numDev;

            if (numLine == 7)
            { //noi facciamo operazione write --> device fa operazione recive =0x0
                //noi facciamo operazione read --> device fa operazione trasmit =0x8

                if ((reg->term.recv_status) != READY)
                {
                    //troviamo lo status del device
                    device_status = reg->term.recv_status;
                    position = position + 8;
                    if (devicesem[position] == -1)
                    { //faccio operazione V

                        pcb_PTR unblocked = sys_v(&devicesem[position]);
                        if (unblocked != NULL)
                        {
                            outProcQ(&readyQ, unblocked);
                            unblocked->p_s.reg_v0 = device_status;
                            //interrupt su subdevice RECIVE perchè != da ready (dopo una nostra richiesta di write)
                            //setto campo COMMAND del device register recive con ACK
                            reg->term.recv_command = ACK;
                            reg->term.recv_status = READY;
                            insertProcQ(&readyQ, unblocked);
                        }
                    }
                }

                if ((reg->term.transm_status) != READY)
                {
                    if (devicesem[position] == -1)
                    { //faccio operazione V
                        pcb_PTR unblocked = sys_v(&devicesem[position]);
                        if (unblocked != NULL)
                        {
                            outProcQ(&readyQ, unblocked);
                            device_status = reg->term.transm_status;
                            unblocked->p_s.reg_v0 = device_status;
                            //interrupt su subdevice TRANSMIT perchè != ready (dopo nostra operazione read)
                            //setto campo COMMAND del device register trasmit con ACK
                            reg->term.transm_command = ACK;
                            reg->term.transm_status = READY;
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
                reg->dtp.status = READY;
                reg->dtp.data0 = 0;
                reg->dtp.data1 = 0;
                device_status = reg->dtp.status;

                if (devicesem[position] == -1)
                {
                    ///faccio operazione V
                    pcb_PTR unblocked = sys_v(&devicesem[position]);

                    if (unblocked != NULL)
                    {
                        outProcQ(&readyQ, unblocked);
                        unblocked->p_s.reg_v0 = device_status;
                        insertProcQ(&readyQ, unblocked);
                    }
                }
            }

            if (current_proc == NULL)
            {
                scheduler();
            }
            else
            {
                LDST((state_t *)BIOSDATAPAGE);
            }
        }
    }
}

void sys_8()
{

    state_t* state_reg = (state_t *)BIOSDATAPAGE;
    state_reg->reg_v0 = (unsigned int)current_proc->p_supportStruct;
    LDST(state_reg);
}

void sys_terminate()
{
    sys_t(current_proc);
    scheduler();
}

void sys_t(pcb_PTR proc)
{   
    state_t* state_reg = (state_t *)BIOSDATAPAGE;

    if (proc != NULL)
    {
        outChild(proc);
        while (!emptyChild(proc))
        {
            sys_t(removeChild(proc));
        }
        if (proc->p_semAdd == NULL)
        {
            outProcQ(&readyQ, proc);
        }
        else
        {
            if (check_dev_semAdd(proc->p_semAdd))
            {
                //è un semaforo device
                softB_count = softB_count - 1;
            }
            else
            {
                *(proc->p_semAdd) = *(proc->p_semAdd) + 1;
            }
            outBlocked(proc);
        }
        freePcb(proc);
        proc_count = proc_count - 1;
    }
}

void sys_p(int *temp)
{
    if (temp == NULL)
    {
        sys_terminate();
    }
    else
    {

        state_t* state_reg = (state_t *)BIOSDATAPAGE;

        *temp = *temp - 1; //decremento risorse
        if (*(temp) < 0)
        {
            //aggiorno stato
            current_proc->p_s = *state_reg;

            if (check_dev_semAdd(temp) == TRUE)
            {
                softB_count = softB_count + 1;
            } //incrementiamo softblocked se è un semAdd di device
            if (insertBlocked(temp, current_proc) == TRUE)
            {
                PANIC();
            } //inserisco il pcb nella coda del semaforo

            //INCREMENTIAMO IL CPU TIME
            int tmp;
            STCK(tmp);
            current_proc->p_time = current_proc->p_time + tmp - count_time;

            scheduler();
        }
        else
        {
            LDST(state_reg);
        }
    }
}

pcb_PTR sys_v(int *temp)
{
    if (temp == NULL)
    {
        sys_terminate();
    }
    else
    {
        state_t* state_reg = (state_t *)BIOSDATAPAGE;

        pcb_PTR newpcb;
        *temp = *temp + 1; //aumento risorse

        //ora c'è una risorsa disponibile quindi sblocco un processo
        newpcb = removeBlocked(temp);


        if (newpcb != NULL) //se è andato tutto bene (removeBlocked ha restituito un pcb)
        {
            newpcb->p_semAdd = NULL;      //pcb non più bloccato
            insertProcQ(&readyQ, newpcb); //lo inserisco nella readyQ

            if (check_dev_semAdd(temp) == TRUE) //se è un semaforo di un DEVICE
            {
                /*  se è NULL significa che il semaforo 'temp' non è più attivo perchè abbiamo tolto 
                     *  l'ultimo pcb bloccato e quindi le sue risorse vengono restettate a 0   */
                if (headBlocked(temp) == NULL)
                {
                    *temp = 0;
                }

                //siccome il pcb era bloccato ad un semaforo dei device devo diminuire soft blocked
                softB_count = softB_count - 1;
            }
        }

        return newpcb;
    }
}

void PassUpOrDie(int EXCEPT)
{
    if (current_proc->p_supportStruct == NULL)
    {
        sys_terminate();
    }
    else
    {
        current_proc->p_supportStruct->sup_exceptState[EXCEPT] = *((state_t *)BIOSDATAPAGE);
        unsigned int sp = current_proc->p_supportStruct->sup_exceptContext[EXCEPT].stackPtr;
        unsigned int status = current_proc->p_supportStruct->sup_exceptContext[EXCEPT].status;
        unsigned int pc = current_proc->p_supportStruct->sup_exceptContext[EXCEPT].pc;
        LDCXT(sp, status, pc);
    }
}


void handlerIT(int numLine){

    //è acceso l'ultmo bit quindi c'è un interrupt pendente sulla linea  2
            numLine = 2;
            LDIT(100000); // setto interval timer di 100 millisecondi (=100000 microsecondi)

            while (headBlocked(&devicesem[48]) != NULL)
            {
                //svuoto la coda dei processi bloccati all' interval timer
                pcb_PTR newpcb = removeBlocked(&devicesem[48]);
                if (newpcb != NULL)
                {
                    newpcb->p_semAdd = NULL;
                    insertProcQ(&readyQ, newpcb);
                    softB_count = softB_count - 1; //diminuisco soft blocked
                    devicesem[48] = devicesem[48] + 1;
                }
            }
            devicesem[48] = 0;

            if (current_proc == NULL)
            {
                scheduler();
            }
            else
            {
                LDST((state_t *)BIOSDATAPAGE);
            }
}
