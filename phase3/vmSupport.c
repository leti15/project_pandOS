#include <vmSupport.h>

void init_spt() {
    for (int i=0; i<POOLSIZE; i++) {
        spt[i].sw_asid = NOPROC;
        spt[i].sw_pageNo = 0;
        spt[i].sw_pte = NULL;
    }
    swp_sem = 1;
}

//da implementare
swap_t* replacement_FIFO(){
    

}

//da rifare
void update_TLB(){

    TLBCLR();
}

//da implementare
void update_BackingStore(){


}

//da implementare
pteEntry_t read_from_backStore(int pageNO){

            /**DA FARE:
         *  1.leggo contenuto pagina (non so come)
         *  2.se errore in lettura -> chiama program trap handler
         */
}

void pager(){
    state_t* state_reg = (state_t *)BIOSDATAPAGE;
    support_t* support_struct = current_proc->p_supportStruct;
    unsigned int cause_reg = support_struct->sup_exceptState[0].cause;

    if(cause_reg != TLBINVLDL && cause_reg != TLBINVLDS){ //invalid modification

        program_trap_exHandle();
    }else{
        sys_p(&swp_sem);
        int missing_page = state_reg->entry_hi;

        swap_t* frame_to_replace = replacement_FIFO();
        if (frame_to_replace->sw_asid != NOPROC){
            //libero frame

        /**DA FARE ATOMICAMENTE*/
            //metto a zero il bit che lo rende invalido
            frame_to_replace->sw_pte->pte_entryLO = frame_to_replace->sw_pte->pte_entryLO & INVALIDMASK;

            update_TLB();
        /**FINO A QUI (atomicamente)*/
            update_BackingStore();
        }
        
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        // leggo contenuto della pagina logica (il cui numero è salvato in frame_to_replace->sw_pageNO) del backing store 
        int virtual_pageNO = frame_to_replace->sw_pageNo;
    
        pteEntry_t backStored_page = read_from_backStore(virtual_pageNO);

        frame_to_replace->sw_asid = current_proc->p_supportStruct->sup_asid;
        frame_to_replace->sw_pageNo = missing_page;
        *frame_to_replace->sw_pte = backStored_page;

    /**DA FARE ATOMICAMENTE*/
        //modifico bit V(page present = 1) e PFN(sta occupando pagina 'frame_to_replace')
        //current_proc->p_supportStruct->sup_privatePgTbl->pte_entryLO = ;

        update_TLB();
    /**FINO A QUI (atomicamente)*/

        sys_v(&swp_sem);
        LDST(state_reg);
    }

}
