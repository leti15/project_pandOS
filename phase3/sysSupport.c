#include "sysSupport.h"

/**     supLevDeviceSem[48] :      
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
char global6;

void general_exHandler(){
    support_t* support_except;
    state_t* state_except;
    support_except = (support_t*)SYSCALL(GETSUPPORTPTR, 0, 0, 0); 
    state_except = (state_t*) &(support_except->sup_exceptState[GENERALEXCEPT]);
    int exCode = state_except->reg_a0;
    
    if (exCode >=9 && exCode <=13){
        syscall_exHandler(exCode);
    }else{
        program_trap_exHandler();
    }
}

void syscall_exHandler(int sysCode){
    support_t* support_except;
    support_except = (support_t*)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    if(sysCode == 9){ //SYSCALL (TERMINATE, 0, 0, 0);
        //rendo invalide le pagine utilizzate dal processo da eliminare
        for (int i = 0; i<MAXPAGES; i += 1)
            support_except->sup_privatePgTbl[i].pte_entryLO = support_except->sup_privatePgTbl[i].pte_entryLO & 0b11111111111111111111110111111111;
        SYSCALL(VERHOGEN, (unsigned int)&masterSEM, 0, 0);
        SYSCALL (TERMPROCESS, 0, 0, 0);
    }
    else if(sysCode == 10){
        //SYSCALL (GETTOD, 0, 0, 0)
        state_t* state_except;
        state_except = (state_t*) &(support_except->sup_exceptState[GENERALEXCEPT]);
        unsigned int time;
        STCK(time);
        state_except->reg_v0 = time; //restituisco in v0

    }
    else if(sysCode == 11){
        sys_11(support_except);
    }
    else if(sysCode == 12){
        sys_12(support_except);
    }
    else if(sysCode == 13){
        sys_13(support_except);
    }
    if(sysCode != 9){
        state_t* state_except;
        state_except = (state_t*) &(support_except->sup_exceptState[GENERALEXCEPT]);
        state_except->pc_epc += 4;
        LDST(state_except);
    }
}

void program_trap_exHandler(){
    swp_sem = 1;
    sys_terminate();
}

void addCharRecvd(char stringa[], int len, char c, char new_stringa[]){
    for (int i = 0; i< len-1; i += 1){
        new_stringa[i] = stringa[i];
    }
    new_stringa[len-1] = c;
    new_stringa[len] = EOS;
}

void sys_11( support_t* support_except){
//SYSCALL (WRITEPRINTER, char *virtAddr,int len, 0)
        /**If the operation ends with a status other than
        “Device Ready” (1), the negative of the device’s status value is returned in v0.
        It is an error to write to a printer device from an address outside of the request-
        ing U-proc’s logical address space, request a SYS11 with a length less than 0, or
        a length greater than 128. Any of these errors should result in the U-proc being
        terminated (SYS9).*/
        state_t* state_except;
        state_except = (state_t*) &(support_except->sup_exceptState[GENERALEXCEPT]);
        if(state_except->reg_a2 < 0 || state_except->reg_a2 > 128 || state_except->reg_a1 < 0x8000000B0 || state_except->reg_a1 > 0xC000000 ){
            SYSCALL (TERMPROCESS, 0, 0, 0);
        }else{
            //parametri corretti, posso procedere
            char stringa[state_except->reg_a2];
            devreg_t* base_regdev = (devreg_t *)GET_devAddrBase(6, support_except->sup_asid);
            int return_msg, i = 0;

            //ottengo mutua esclsione sul device register
            SYSCALL(PASSEREN, (unsigned int)&devRegSem[support_except->sup_asid], 0, 0);

            while ( i < state_except->reg_a2 && stringa[i] != EOS && return_msg == READY)
            {
                base_regdev->dtp.data0 = stringa[i];
                base_regdev->dtp.command = 2; //= PRINTCHAR
                return_msg = SYSCALL(IOWAIT, 6, support_except->sup_asid, FALSE);
                i += 1;
            }

            //rilascio mutua esclusione sul semaforo del device register
            SYSCALL(VERHOGEN, (unsigned int)&devRegSem[support_except->sup_asid], 0, 0);

            if (return_msg != READY){
                //qualcosa è andato storto
                state_except->reg_v0 = return_msg;
            }else{
                //stringa stampata correttamente
                state_except->reg_v0 = i; //numero caratteri stampati (non conto l'EOS)
            }

        }
}

void sys_12( support_t* support_except){
        //SYSCALL (WRITETERMINAL, char *virtAddr,int len, 0)
        state_t* state_except;
        state_except = (state_t*) &(support_except->sup_exceptState[GENERALEXCEPT]);
        if(state_except->reg_a2 >= 0 && state_except->reg_a2 <= 128 && state_except->reg_a1 >= VPNBASE && state_except->reg_a1 <= USERSTACKTOP){
            //parametri corretti, posso procedere
            char* stringa = (char*) state_except->reg_a1;
            int position = 32 + support_except->sup_asid - 1;
            devreg_t* base_regdev = (devreg_t*)(0x10000054 + (4 * 0x80) + ((support_except->sup_asid - 1) * 0x10));
            int return_msg, i = 0;
            //ottengo mutua esclsione sul device register
            SYSCALL(PASSEREN,(unsigned int) &supLevDeviceSem[position], 0, 0);

            //while ( i < state_except->reg_a2 && stringa[i] != EOS && (return_msg == READY || return_msg == 5))
            while ( i < state_except->reg_a2)
            {
                atomON();
                int command = (unsigned int) (*stringa << 8) + TRANSMITCHAR;
                base_regdev->term.transm_command = command;
                return_msg = SYSCALL(IOWAIT, 7, support_except->sup_asid - 1, FALSE);
                atomOFF();
                stringa += 1;
                i += 1;
            }

            //rilascio mutua esclusione sul semaforo del device register
            SYSCALL(VERHOGEN, (unsigned int) &supLevDeviceSem[position], 0, 0);

            return_msg &= 0xFF; //elimino la parte transmit char e mi rimane solo la parte transmit status
            if (return_msg != READY && return_msg != 5){
                //qualcosa è andato storto
                state_except->reg_v0 = return_msg;
            }else{
                //stringa stampata correttamente
                state_except->reg_v0 = i; //numero caratteri trasmessi (non conto l'EOS)
            }
        } else{
            SYSCALL (TERMPROCESS, 0, 0, 0);
        }


}

void sys_13( support_t* support_except){
        //SYSCALL (READTERMINAL, char *virtAddr,0, 0)
        state_t* state_except;
        state_except = (state_t*) &(support_except->sup_exceptState[GENERALEXCEPT]);
        if( state_except->reg_a1 < VPNBASE || state_except->reg_a1 > USERSTACKTOP ){
            SYSCALL (TERMPROCESS, 0, 0, 0);
        }else{
            //parametri corretti, posso procedere
            char* stringa = (char*) state_except->reg_a1;
            char rcv_char;
            int position = 40 + support_except->sup_asid - 1;
            devreg_t* base_regdev = (devreg_t*)(0x10000054 + (4 * 0x80) + ((support_except->sup_asid - 1) * 0x10));
            int return_msg, counter_char = 0;

            //ottengo mutua esclsione sul device register
            SYSCALL(PASSEREN, (unsigned int)&supLevDeviceSem[position], 0, 0);

            //while ((return_msg == READY || return_msg == 5) && rcv_char != EOS)
            int statusBIT = base_regdev->term.recv_status & RECVSTATUSBIT;
            while (TRUE)
            {  
                atomON();
                base_regdev->term.recv_command = TRANSMITCHAR;
                return_msg = SYSCALL(IOWAIT, 7, support_except->sup_asid - 1, TRUE);
                
                statusBIT = base_regdev->term.recv_status & RECVSTATUSBIT;
                rcv_char = return_msg >> 8;
                atomOFF();
                if (rcv_char == '\n'){
                    break;
                }
                *stringa = rcv_char;
                stringa++;
                //*(stringa + counter_char * sizeof(char)) = rcv_char;
                counter_char += 1;
            }

            //rilascio mutua esclusione sul semaforo del device register
            SYSCALL(VERHOGEN, (unsigned int)&supLevDeviceSem[position], 0, 0);
            global4 = statusBIT;
            /*if (statusBIT != 5 && statusBIT != 1){ //qualcosa è andato storto
                state_except->reg_v0 = statusBIT;
            }else{ //stringa stampata correttamente*/

                state_except->reg_v0 = counter_char; //numero caratteri ricevuti (sottraggo il carattere EOS)
            //}
        }
}


