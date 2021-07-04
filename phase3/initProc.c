#include "initProc.h"
int devRegSem[8];

void test(){
    init_spt();
    init_devices();
    init_supLevSem();

    static state_t childrenState[8];
    for (int i = 1; i < 9; i += 1){
        childrenState[i].entry_hi = i; //setto id processo
        childrenState[i].pc_epc = 0x800000B0;
        childrenState[i].reg_t9 = 0x800000B0;
        childrenState[i].reg_sp = 0xC0000000;
        childrenState[i].status = 0b00011000000000001111111100000011;//interrupts (anche local timer) abilitati e user mode
    }

    static support_t childrenSupport[8];
    for (int i = 1; i <= 8; i += 1){
        childrenSupport[i].sup_asid = i;
        childrenSupport[i].sup_exceptContext[PGFAULTEXCEPT].pc = (memaddr) uTLB_RefillHandler;
        childrenSupport[i].sup_exceptContext[GENERALEXCEPT].pc = (memaddr) general_exHandler; 
        childrenSupport[i].sup_exceptContext[PGFAULTEXCEPT].status = 0b00011000000000001111111100000001;//interrupts (anche local timer) abilitati e kernel mode
        childrenSupport[i].sup_exceptContext[GENERALEXCEPT].status = 0b00011000000000001111111100000001;//interrupts (anche local timer) abilitati e kernel mode
        /**Stacks grow “down” so set the SP fields to the address of the end
         * of these areas. e.g. . . . = &(. . .sup stackGen[499])*/
        childrenSupport[i].sup_exceptContext[PGFAULTEXCEPT].stackPtr = PUV->tlb_refill_stackPtr;
        childrenSupport[i].sup_exceptContext[GENERALEXCEPT].stackPtr = PUV->exception_stackPtr;
        initPGTBL(childrenSupport[i].sup_privatePgTbl, i);
    }

    for (int i = 1; i <= 8; i += 1){
        //i-esimo processo figlio
        SYSCALL(CREATEPROCESS, childrenState[i], childrenSupport[i], 0);
    }

    if(TRUE)
        SYSCALL(TERMPROCESS, 0, 0, 0);
    else
        SYSCALL(PASSEREN, devicesem[48], 0, 0);

}

void initPGTBL(pteEntry_t* pgtable, int asid){
    for (int i = 0; i < MAXPAGES-1; i += 1){
        pgtable[i].pte_entryHI = ((0x80000 + i) << 12) | asid;
        pgtable[i].pte_entryLO = 0b00000000000000000000010100000000; //( D bit = 1, V bit = 0, G bit = 1 ) 
    }
    //stack page
    pgtable[MAXPAGES].pte_entryHI = ((0xBFFFF + MAXPAGES-1) << 12) | asid;
    pgtable[MAXPAGES].pte_entryLO = 0b00000000000000000000010100000000; //( D bit = 1, V bit = 0, G bit = 1 ) 
}

void init_supLevSem(){
    for (int i = 0; i <= 8; i += 1)
        devRegSem[i] = 1;
}