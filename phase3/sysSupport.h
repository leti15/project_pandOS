#ifndef SYSSUPPORT_H
#define SYSSUPPORT_H

#include "pcb.h"
#include "asl.h"
#include "pandos_types.h"
#include "commons.h"
#include "scheduler.h"
#include "vmSupport.h"

void bp();
void general_exHandler();
void syscall_exHandler(int sysCode);
void program_trap_exHandler();
char* addCharRecvd(char stringa[], int len, char c, char new_stringa[]);

#endif