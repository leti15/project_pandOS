#include <vmSupport.h>

void init_spt() {
    for (int i=0; i<POOLSIZE; i++) {
        spt[i].sw_asid = NOPROC;
        spt[i].sw_pageNo = 0;
        spt[i].sw_pte = NULL;
    }
    swp_sem = 1;
}

swap_t* replacement_FIFO(){
    static int chosenframe = - 1;
    chosenframe = chosenframe + 1;
    return (swap_t*) spt + (chosenframe%POOLSIZE);
}

//da rifare
void update_TLB(){

    TLBCLR();
}

//da implementare
void update_BackingStore(swap_t* frame, int owner_process){
 /**DA FARE:
         *  1.leggo contenuto pagina (non so come)
         *  2.se errore in lettura -> chiama program trap handler
         * 
         */


}

/**
 * se 'readMode' è 1, legge il contenuto della pagina fisica relativa alla pagina virtuale 'virtualPG' (dalla page table da 'support_process') e lo salva in 'frame';
 * se 'readmode' è 0, scrive il contenuto di 'frame' nella pagina fisica relativa alla pagina virtuale 'virtualPG' (della page table da 'support_process')
 * NB: se PFN != -1 allora invece di calcolare l'indirizzo del frame fisico dalla page table semplicemente usiamo quello 
*/
unsigned int ReadWrite_from_backStore(swap_t* frame, pteEntry_t* page_table, int processID, int virtualPG, unsigned int PFN, int readMode){

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

        //trovo frame fisico corrispondente alla pagina virtuale
        if (PFN == -1)
        {   
            for (int i = 0; i < USERPGTBLSIZE; i+=1){
                if( page_table[i].pte_entryHI >> 12 == virtualPG)
                    PFN = page_table[i].pte_entryLO >> 12;
            }
        }
        //se trovo la pagina procedo, altrimenti errore!
        if(PFN != -1)
        {   int device_num = processID; //penso sia giusto cosi....(non sono sicura)
            devreg_t* base = ((unsigned int)GET_devAddrBase(4, device_num));//=(Nlinea, Ndevice)

            //scrivo campo DATA0
            base->dtp.data0 = ((unsigned int)*frame) >> 12;  //indirizzo del frame shiftato di 12

        /**DA FARE ATOMICAMENTE= disabilitare interrupts*/
            atomON();

            //scrivo campo COMMAND
            base->dtp.command = PFN & 0b11111111111111111111000000000000; // == BLOCKNUMBER??
            if (readMode == 1) //READ
                base->dtp.command = base->dtp.command | 0b00000000000000000000000000000010;
            else //WRITE
                base->dtp.command = base->dtp.command | 0b00000000000000000000000000000011;

            //chiamo syscall 5
            SYSCALL(IOWAIT, 4, device_num, FALSE); //se restituisce errore --> chiama program trap handler
        /**FINO A QUI (atomicamente)*/
            atomOFF();

        }else{
            //ERRORE!
        }
}

void pager(){
    state_t* state_reg = (state_t *)BIOSDATAPAGE;
    support_t* support_struct = current_proc->p_supportStruct;
    unsigned int cause_reg = support_struct->sup_exceptState[0].cause;

    if(cause_reg != TLBINVLDL && cause_reg != TLBINVLDS){ //invalid modification

        program_trap_exHandle();
    }else{
        sys_p(&swp_sem);
        int missing_page_entryHI = state_reg->entry_hi; //campo entryHI contenente la pagina virtuale che ha causato il page fault
        int p = inspecteHI(missing_page_entryHI); //numero pagina virtuale non trovata (che ha causato page fault)
        
        swap_t* frame_to_replace = replacement_FIFO(); // = i
        int old_owner = frame_to_replace->sw_asid; ;//= x = processo proprietario della pagina nel frame da liberare
        int vPN = frame_to_replace->sw_pageNo;   //= k = virtual page number da liberare

        if (frame_to_replace->sw_asid != NOPROC){
            //libero frame
            if ((frame_to_replace->sw_pte->pte_entryLO & 0b00000000000000000000001000000000) == 0){  //se il dirty bit ("V") == 0 allora NOTvalid

            /**DA FARE ATOMICAMENTE= disabilitare interrupts*/
                atomON();
                //metto a zero il bit che lo rende invalido ovvero "V"
                frame_to_replace->sw_pte->pte_entryLO = frame_to_replace->sw_pte->pte_entryLO & INVALIDMASK;
                update_TLB();
            /**FINO A QUI (atomicamente)*/
                atomOFF();
                
                /*aggiorno la page table ed il backing store del processo al quale sto "fregando" 
                * il frame da usare per la pagina virtuale che ho cercato ma che non ho trovato
                *    "Update process x’s backing store. Write the contents of frame i to the
                *    correct location on process x’s backing store/flash device."
                */
                //update_BackingStore(frame_to_replace, old_owner);  
                unsigned int pFN = (frame_to_replace->sw_pte->pte_entryLO) >> 12;
                ReadWrite_from_backStore (frame_to_replace, NULL, old_owner, vPN, pFN, 0);
            }
        }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /**
         * "Read the contents of the Current Process’s backing store/flash device logical
            page p into frame i. [Section 4.5.1]
            Treat any error status from the read operation as a program trap.""
        */
        ReadWrite_from_backStore(frame_to_replace, support_struct->sup_privatePgTbl, old_owner, -1, p, 1);


        //updating swap pool 
        frame_to_replace->sw_pageNo = p;  //TODO: da ricontrollare
        frame_to_replace->sw_asid = support_struct->sup_asid;

    /**DA FARE ATOMICAMENTE = disabilitare interrupts*/
        atomON();
        /**Update the Current Process’s Page Table entry for page p to indicate it is
        now present (V bit) and occupying frame i (PFN field).*/

        //modifico bit V(page present = 1) e PFN(sta occupando pagina 'frame_to_replace')




        /**Update the TLB. The cached entry in the TLB for the Current Process’s
        page p is clearly out of date; it was just updated in the previous step.*/
        update_TLB();
        
    /**FINO A QUI (atomicamente)*/
        atomOFF();

        sys_v(&swp_sem);
        LDST(state_reg);
    }

}
