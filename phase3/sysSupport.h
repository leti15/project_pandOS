#ifndef SYSSUPPORT_H
#define SYSSUPPORT_H

#include "pcb.h"
#include "asl.h"
#include "pandos_types.h"
#include "commons.h"
#include "scheduler.h"
#include "vmSupport.h"

extern int supLevDeviceSem[48];
extern int masterSEM;
extern void pager();

void general_exHandler();
void syscall_exHandler(int sysCode);
void program_trap_exHandler();
void addCharRecvd(char stringa[], int len, char c, char new_stringa[]);
void sys_9( support_t* support_except);
void sys_10( support_t* support_except);
void sys_11( support_t* support_except);
void sys_12( support_t* support_except);
void sys_13( support_t* support_except);

#endif