#include "initProc.h"
int devRegSem[8];
int masterSEM;

extern void bp();
extern void bp1();
extern void bp2();
extern void bp3();
extern void b4();
extern void b5();
extern void b6();
extern void boo();

void test(){
    init_spt();
    init_devices();
    init_supLevSem();
    

    static state_t childrenState[8];
    for (int i = 1; i < 9; i += 1){
        childrenState[i].entry_hi = i << ASIDSHIFT; //setto id processo
        childrenState[i].pc_epc = 0x800000B0;
        childrenState[i].reg_t9 = 0x800000B0;
        childrenState[i].reg_sp = 0xC0000000;
        childrenState[i].status = IMON | IEPON | USERPON | TEBITON;//interrupts (anche local timer) abilitati e user mode
    }

    static support_t childrenSupport[8];
    for (int i = 1; i <= 8; i += 1){
        childrenSupport[i].sup_asid = i;
        childrenSupport[i].sup_exceptContext[PGFAULTEXCEPT].pc = (memaddr) pager;
        childrenSupport[i].sup_exceptContext[GENERALEXCEPT].pc = (memaddr) general_exHandler; 
        childrenSupport[i].sup_exceptContext[PGFAULTEXCEPT].status = IMON | IEPON | TEBITON;//interrupts (anche local timer) abilitati e kernel mode
        childrenSupport[i].sup_exceptContext[GENERALEXCEPT].status = IMON | IEPON | TEBITON;//interrupts (anche local timer) abilitati e kernel mode
        childrenSupport[i].sup_exceptContext[PGFAULTEXCEPT].stackPtr = &(childrenSupport[i].sup_stackTLB[499]);
        childrenSupport[i].sup_exceptContext[GENERALEXCEPT].stackPtr = &(childrenSupport[i].sup_stackGen[499]);
        initPGTBL(childrenSupport[i].sup_privatePgTbl, i);
    }


    bp();
    for (int i = 1; i <= 8; i += 1){
        //i-esimo processo figlio
       bp1();
        SYSCALL(CREATEPROCESS, &(childrenState[i]), &(childrenSupport[i]), 0);
    }
    bp();
    for (int i = 1; i <= 8; i += 1){
        SYSCALL(PASSEREN, &masterSEM, 0, 0);
    }


/*
    if(TRUE)
        SYSCALL(TERMPROCESS, 0, 0, 0);
    else
        SYSCALL(PASSEREN, devicesem[48], 0, 0);
*/
}

void initPGTBL(pteEntry_t* pgtable, int asid){
    for (int i = 0; i < MAXPAGES-1; i += 1){
        pgtable[i].pte_entryHI = ((0x80000 + i) << 12) | asid << ASIDSHIFT;
        pgtable[i].pte_entryLO = 0b00000000000000000000010000000000; //( D bit = 1, V bit = 0, G bit = 0 ) 
    }
    //stack page
    pgtable[MAXPAGES].pte_entryHI = (0xBFFFF << 12) | asid << ASIDSHIFT;
    pgtable[MAXPAGES].pte_entryLO = 0b00000000000000000000010000000000; //( D bit = 1, V bit = 0, G bit = 0 ) 
}

void init_supLevSem(){
    for (int i = 0; i <= 8; i += 1)
        devRegSem[i] = 1;
    masterSEM = 0;
}