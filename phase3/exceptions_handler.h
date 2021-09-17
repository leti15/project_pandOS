#ifndef EXCEPTIONS_HANDLER_H
#define EXCEPTIONS_HANDLER_H

#include "pcb.h"
#include "asl.h"
#include "pandos_types.h"
#include "commons.h"
#include "scheduler.h"

extern int resource_dev;
extern int resource_it;

//void uTLB_RefillHandler();
void exception_handler();
void SYS_handler();
int interrupt_handler();
void sys_8();
void sys_terminate ();
void sys_t(pcb_PTR proc);
void sys_p (int* temp);
pcb_PTR sys_v (int* temp);
void PassUpOrDie(int EXCEPT);
void handlerIT(int numLine);

#endif /* !defined(EXCEPTIONS_HANDLER_H) */