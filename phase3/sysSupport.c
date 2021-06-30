#include <sysSupport.h>

support_t* support_except;
state_t* state_except;

void general_exHandle(){
    state_except = (state_t *)BIOSDATAPAGE;
    int exCode = CAUSE_GET_EXCCODE(current_proc->p_supportStruct->sup_exceptState->cause);//nonn so se va bene o no prenderlo dal current process
    if (exCode >=9 && exCode <=13){
        syscall_exHandle(exCode);
    }else{
        program_trap_exHandle();
    }
}

void syscall_exHandle(int sysCode){
    support_except = SYSCALL(GETSUPPORTPTR, 0, 0, 0); 
    /** oppure non si può usare "SYSCALL"??
     *  sys_8();
     *  support_except = state_except->reg_v0;
    */
    state_except = (state_t*) &(current_proc->p_supportStruct->sup_exceptState[GENERALEXCEPT]);

    //incrementare PC di 4....quale dei due??
    current_proc->p_s.pc_epc += 4;
    state_except->pc_epc += 4;//penso questo

    if(sysCode == 9){
        //SYSCALL (TERMINATE, 0, 0, 0)
        sys_terminate();
        
    }else if(sysCode == 10){
        //SYSCALL (GETTOD, 0, 0, 0)
        unsigned int time;
        STCK(time);
        state_except->reg_v0 = time; //restituisco in v0

    }else if(sysCode == 11){
        //SYSCALL (WRITEPRINTER, char *virtAddr,int len, 0)
        /**If the operation ends with a status other than
        “Device Ready” (1), the negative of the device’s status value is returned in v0.
        It is an error to write to a printer device from an address outside of the request-
        ing U-proc’s logical address space, request a SYS11 with a length less than 0, or
        a length greater than 128. Any of these errors should result in the U-proc being
        terminated (SYS9).*/
        if(state_except->reg_a2 < 0 || state_except->reg_a2 > 128 || state_except->reg_a1 == /*address out of virtual space*/){
            sys_terminate();
        }else{
            //parametri corretti, posso procedere
            SYSCALL(IOWAIT, 6, support_except->sup_asid, 0);

        }
    }else if(sysCode == 12){
        //SYSCALL (WRITETERMINAL, char *virtAddr,int len, 0)
    }else if(sysCode == 13){
        //SYSCALL (READTERMINAL, char *virtAddr,0, 0)
    }


}

void program_trap_exHandle(){}