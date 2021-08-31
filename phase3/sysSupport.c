#include "sysSupport.h"

support_t* support_except;
state_t* state_except;
extern int supLevDeviceSem[48];
extern int masterSEM;
extern void pager();

void bp(){};
void bp1(){};
void bp2(){};
void bp3(){};
void b4(){};
void b5(){};
void b6(){};
void boo(){};

int global4;
int global5;

void general_exHandler(){
    support_except = SYSCALL(GETSUPPORTPTR, 0, 0, 0); 
    state_except = (state_t*) &(support_except->sup_exceptState[GENERALEXCEPT]);

    //int exCode = CAUSE_GET_EXCCODE(state_except->cause);//non so se va bene o no prenderlo dal current process
    int exCode = state_except->reg_a0;
    
    if (exCode >=9 && exCode <=13){
        syscall_exHandler(exCode);
    }else{
        program_trap_exHandler();
    }
}

void syscall_exHandler(int sysCode){
    support_except = SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    if(sysCode == 9){ //SYSCALL (TERMINATE, 0, 0, 0);
        //rendo invalide le pagine utilizzate dal processo da eliminare
        for (int i = 0; i<MAXPAGES; i += 1)
            support_except->sup_privatePgTbl[i].pte_entryLO = support_except->sup_privatePgTbl[i].pte_entryLO & 0b11111111111111111111110111111111;
        SYSCALL(VERHOGEN, &masterSEM, 0, 0);
        SYSCALL (TERMPROCESS, 0, 0, 0);
    }
    else if(sysCode == 10){
        //SYSCALL (GETTOD, 0, 0, 0)
        unsigned int time;
        STCK(time);
        state_except->reg_v0 = time; //restituisco in v0

    }
    else if(sysCode == 11){
        //SYSCALL (WRITEPRINTER, char *virtAddr,int len, 0)
        /**If the operation ends with a status other than
        “Device Ready” (1), the negative of the device’s status value is returned in v0.
        It is an error to write to a printer device from an address outside of the request-
        ing U-proc’s logical address space, request a SYS11 with a length less than 0, or
        a length greater than 128. Any of these errors should result in the U-proc being
        terminated (SYS9).*/
        if(state_except->reg_a2 < 0 || state_except->reg_a2 > 128 || state_except->reg_a1 < 0x8000000B0 || state_except->reg_a1 > 0xC000000 ){
            SYSCALL (TERMPROCESS, 0, 0, 0);
        }else{
            //parametri corretti, posso procedere
            char stringa[state_except->reg_a2];
            devreg_t* base_regdev = GET_devAddrBase(6, support_except->sup_asid);
            int return_msg, i = 0;

            //ottengo mutua esclsione sul device register
            SYSCALL(PASSEREN, &devRegSem[support_except->sup_asid], 0, 0);

            while ( i < state_except->reg_a2 && stringa[i] != EOS && return_msg == READY)
            {
                base_regdev->dtp.data0 = stringa[i];
                base_regdev->dtp.command = 2; //= PRINTCHAR
                return_msg = SYSCALL(IOWAIT, 6, support_except->sup_asid, FALSE);
                i += 1;
            }

            //rilascio mutua esclusione sul semaforo del device register
            SYSCALL(VERHOGEN, &devRegSem[support_except->sup_asid], 0, 0);

            if (return_msg != READY){
                //qualcosa è andato storto
                state_except->reg_v0 = return_msg;
            }else{
                //stringa stampata correttamente
                state_except->reg_v0 = i; //numero caratteri stampati (non conto l'EOS)
            }

        }
    }
    else if(sysCode == 12){
        //SYSCALL (WRITETERMINAL, char *virtAddr,int len, 0)
        boo();
        if(state_except->reg_a2 >= 0 && state_except->reg_a2 <= 128 && state_except->reg_a1 >= VPNBASE && state_except->reg_a1 <= USERSTACKTOP){
            //parametri corretti, posso procedere
            global5= state_except->reg_a1;
            bp1();
            char* stringa = (char*) state_except->reg_a1;
            bp2();
            devreg_t* base_regdev = GET_devAddrBase(7, support_except->sup_asid - 1);
            bp3();
            int return_msg, i = 0;
            bp();
            //ottengo mutua esclsione sul device register
            SYSCALL(PASSEREN, &supLevDeviceSem[32 + support_except->sup_asid - 1], 0, 0);

            //while ( i < state_except->reg_a2 && stringa[i] != EOS && (return_msg == READY || return_msg == 5))
            while ( i < state_except->reg_a2)
            {
                atomON();
                global4= base_regdev;
                global5 = (*stringa<< 8) | 2;
                b4();
                base_regdev->term.transm_command = (unsigned int) (*stringa << 8) | TRANSMITCHAR;
                b5();
                return_msg = SYSCALL(IOWAIT, 7, support_except->sup_asid - 1, FALSE);
                boo();
                atomOFF();
                stringa += 1;
                i += 1;
            }

            //rilascio mutua esclusione sul semaforo del device register
            SYSCALL(VERHOGEN, &supLevDeviceSem[32 + support_except->sup_asid - 1], 0, 0);

            return_msg &= 0xFF; //elimino la parte transmit char e mi rimane solo la parte transmit status
            if (return_msg != READY && return_msg != 5){
                //qualcosa è andato storto
                state_except->reg_v0 = return_msg;
            }else{
                //stringa stampata correttamente
                state_except->reg_v0 = i; //numero caratteri trasmessi (non conto l'EOS)
            }
        } else{
            global5 = state_except->reg_a1;
            SYSCALL (TERMPROCESS, 0, 0, 0);
        }
    }
    else if(sysCode == 13){
        //SYSCALL (READTERMINAL, char *virtAddr,0, 0)
         if( state_except->reg_a1 < 0x8000000B0 || state_except->reg_a1 > 0xC000000 ){
            SYSCALL (TERMPROCESS, 0, 0, 0);
        }else{
            //parametri corretti, posso procedere
            char initial_stringa = EOS;
            char *stringa = &initial_stringa;
            char rcv_char = '0';
            devreg_t* base_regdev = GET_devAddrBase(6, support_except->sup_asid);
            int return_msg, i = 1;

            //ottengo mutua esclsione sul device register
            SYSCALL(PASSEREN, &devRegSem[support_except->sup_asid], 0, 0);

            while ((return_msg == READY || return_msg == 5) && rcv_char != EOS)
            {   char new_stringa[i+1];
                base_regdev->term.recv_command = 2;
                return_msg = SYSCALL(IOWAIT, 7, support_except->sup_asid - 1, TRUE);
                rcv_char = base_regdev->term.recv_status >> 8;
                stringa = addCharRecvd(stringa, i, rcv_char, new_stringa);
                i += 1;
            }

            //rilascio mutua esclusione sul semaforo del device register
            SYSCALL(VERHOGEN, &devRegSem[support_except->sup_asid], 0, 0);

            if (return_msg != 5){
                //qualcosa è andato storto
                state_except->reg_v0 = return_msg;
            }else{
                //stringa stampata correttamente
                state_except->reg_v0 = i-1; //numero caratteri ricevuti (sottraggo il carattere EOS)
            }
        }
    }
    if(sysCode != 9){
        state_except->pc_epc += 4;
        LDST(state_except);
    }
}

void program_trap_exHandler(){
    swp_sem = 1;
    sys_terminate();
}

char* addCharRecvd(char stringa[], int len, char c, char new_stringa[]){
    for (int i = 0; i< len-1; i += 1){
        new_stringa[i] = stringa[i];
    }
    new_stringa[len-1] = c;
    new_stringa[len] = EOS;
    return new_stringa;

}