#ifndef EXCEPTIONS_HANDLER_H
#define EXCEPTIONS_HANDLER_H

#include "pcb.h"
#include "asl.h"
#include "pandos_types.h"
#include "commons.h"
#include "scheduler.h"

extern int resource_dev;
extern int resource_it;

void fooBar();
void SYS_handler();
int interrupt_handler();
void sys_terminate ();
void sys_p (int* temp);
pcb_PTR sys_v (int* temp);
void PassUpOrDie(int EXCEPT);

#endif /* !defined(EXCEPTIONS_HANDLER_H) */