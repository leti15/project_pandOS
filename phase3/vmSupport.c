#include "vmSupport.h"

swap_t  spt[2*Nproc];
int swp_sem;
state_t* state_reg;
support_t* support_struct;

int missing_global;

void uTLB_RefillHandler(){
    state_reg = (state_t *)BIOSDATAPAGE;

    bp();

    int missing_page = state_reg->entry_hi;
    if (missing_page <= 0xC0000000 && missing_page >= STACKPG)
        missing_page = 31;
    else
        missing_page = ( state_reg->entry_hi - VPNBASE ) >> 12;
        
    missing_global = missing_page;
    bp1();

    pteEntry_t new_pgEntry = current_proc->p_supportStruct->sup_privatePgTbl[missing_page];

    bp2();

    missing_global = new_pgEntry.pte_entryHI;
    setENTRYHI( (unsigned int) new_pgEntry.pte_entryHI);
    setENTRYLO( (unsigned int) new_pgEntry.pte_entryLO);
    TLBWR();

    LDST(state_reg);
}


void init_spt() {
    for (int i = 0; i < POOLSIZE; i += 1) {
        spt[i].sw_asid = NOPROC;
        //spt[i].sw_pageNo = 0;
        //spt[i].sw_pte = NULL;
    }
    swp_sem = 1;
}

int replacement_FIFO(){
    static int chosenframe = 0;
    return (chosenframe++)%MAXPAGES;
}

//da rifare
void update_TLB(){

    TLBCLR();
}

/**
 * se 'readMode' è 1, legge il contenuto della pagina fisica relativa alla pagina virtuale 'virtualPG' (dalla page table da 'support_process') e lo salva in 'frame';
 * se 'readmode' è 0, scrive il contenuto di 'frame' nella pagina fisica relativa alla pagina virtuale 'virtualPG' (della page table da 'support_process')
 * NB: se PFN != -1 allora invece di calcolare l'indirizzo del frame fisico dalla page table semplicemente usiamo quello 
*/
unsigned int ReadWrite_from_backStore(int processID, int blocknumber, unsigned int to_write_or_read, int readMode){

        /**DA FARE:
         *  1.leggo contenuto pagina (non so come)
         *  2.se errore in lettura -> chiama program trap handler
         * 1.Write the flash device’s DATA0 field with the appropriate starting physical
            address of the 4k block to be read (or written); the particular frame’s starting
            address.
          2.Write the flash device’s COMMAND field with the device block number
            (high order three bytes) and the command to read (or write) in the lower
            order byte.
         As with all I/O operations, this should be immediately followed by a SYS5
         */

        
        int device_num = processID; //penso sia giusto cosi....(non sono sicura)
        devreg_t* base = ((unsigned int) GET_devAddrBase(4, device_num));//=(Nlinea, Ndevice)
            
        //ottengo mutua esclusione sul device register
        SYSCALL(PASSEREN, &devRegSem[processID], 0, 0);

        //scrivo campo DATA0
        base->dtp.data0 = to_write_or_read; 

    /**DA FARE ATOMICAMENTE= disabilitare interrupts*/
        atomON();

        //scrivo campo COMMAND
        if (readMode == 1) //READ
            base->dtp.command = (blocknumber << 8) | FLASHREAD;
        else //WRITE
            base->dtp.command = (blocknumber << 8) | FLASHWRITE;

        //chiamo syscall 5
        SYSCALL(IOWAIT, 4, device_num, FALSE); //se restituisce errore --> chiama program trap handler
    /**FINO A QUI (atomicamente)*/
        atomOFF();

        //rilascio semaforo device register
        SYSCALL(VERHOGEN, &devRegSem[processID], 0, 0);

        if (base->dtp.status != READY) 
            SYSCALL(TERMINATE, 0, 0, 0);
}

int global3;
void pager(){
    state_reg = (state_t *)BIOSDATAPAGE;
    support_struct = SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    unsigned int cause_reg = (support_struct->sup_exceptState[0].cause & GETEXECCODE) >> CAUSESHIFT;

    if(cause_reg == 1){ //invalid modification

        program_trap_exHandler();
    }else{
        SYSCALL(PASSEREN, &swp_sem, 0, 0);
        int missing_page_entryHI = support_struct->sup_exceptState[PGFAULTEXCEPT].entry_hi; //campo entryHI contenente la pagina virtuale che ha causato il page fault
        int p;
        if (missing_page_entryHI >= STACKPG && missing_page_entryHI <= USERSTACKTOP)
            p = 31;
        else
            p = (missing_page_entryHI - VPNBASE) >> VPNSHIFT; //numero pagina virtuale non trovata (che ha causato page fault)
            
        
        int address_data;
        int frame_to_replace = replacement_FIFO(); // = i
        global3 = frame_to_replace;

        int old_owner = spt[frame_to_replace].sw_asid; //= x = processo proprietario della pagina nel frame da liberare

        if (spt[frame_to_replace].sw_pte->pte_entryLO & VALIDON){
            boo();
            //libero frame perchè la pagina era valida
            int vPN = spt[frame_to_replace].sw_pageNo;   //= k = virtual page number da liberare

            spt[frame_to_replace].sw_pte->pte_entryLO &= ~(0b00000000000000000000001000000000);//dirty bit ("V") == 0 = NOTvalid

            /**DA FARE ATOMICAMENTE= disabilitare interrupts*/
                atomON();
                //metto a zero il bit che lo rende invalido ovvero "V"
                spt[frame_to_replace].sw_pte->pte_entryLO = spt[frame_to_replace].sw_pte->pte_entryLO & INVALIDbitV;
                update_TLB();
            /**FINO A QUI (atomicamente)*/
                atomOFF();
                
                /*aggiorno la page table ed il backing store del processo al quale sto "fregando" 
                * il frame da usare per la pagina virtuale che ho cercato ma che non ho trovato
                *    "Update process x’s backing store. Write the contents of frame i to the
                *    correct location on process x’s backing store/flash device."
                */
                address_data = SWAP_POOL_START + (PAGESIZE * frame_to_replace);
                ReadWrite_from_backStore ( old_owner - 1, vPN, address_data, 0); //scrivo nel backing store del processo del vecchio frame
        }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /**
         * "Read the contents of the Current Process’s backing store/flash device logical
            page p into frame i. [Section 4.5.1]
            Treat any error status from the read operation as a program trap.""
        */
 
        address_data = SWAP_POOL_START + (PAGESIZE * frame_to_replace);
        global3 = address_data;
        //leggere dal backing store del processo che ha causato il page fault e caricare in memoria la pagina mancante
        ReadWrite_from_backStore ( support_struct->sup_asid - 1, p, address_data, 1); 

        bp1();

    /**DA FARE ATOMICAMENTE = disabilitare interrupts*/
        atomON();

        bp2();

        //updating swap pool 
        /*
        int pos_p;
        for (int i = 0; i < USERPGTBLSIZE; i+= 1)
            if((support_struct->sup_privatePgTbl[i].pte_entryHI >> 12) == p){
                spt[frame_to_replace].sw_pte = &(support_struct->sup_privatePgTbl[i]);
                pos_p = i;
            }
            */
        spt[frame_to_replace].sw_pte = &(support_struct->sup_privatePgTbl[p]);
        spt[frame_to_replace].sw_pageNo = p; 
        spt[frame_to_replace].sw_asid = support_struct->sup_asid;//segnalo che il nuovo processo sta ora occupando 'frame_to_replace'
        
        bp3();

        /**Update the Current Process’s Page Table entry for page p to indicate it is
        now present (V bit) and occupying frame i (PFN field).*/
        //modifico bit V(page present = 1) 
        //support_struct->sup_privatePgTbl[pos_p].pte_entryLO = support_struct->sup_privatePgTbl[pos_p].pte_entryLO | VALIDbitV;
        support_struct->sup_privatePgTbl[p].pte_entryLO = address_data | VALIDON | DIRTYON;
        
        /**Update the TLB. The cached entry in the TLB for the Current Process’s
        page p is clearly out of date; it was just updated in the previous step.*/
        update_TLB();
        
    /**FINO A QUI (atomicamente)*/
        atomOFF();

        SYSCALL(VERHOGEN, & swp_sem, 0, 0);
        LDST((state_t*) &support_struct->sup_exceptState[PGFAULTEXCEPT]);
    }

}


