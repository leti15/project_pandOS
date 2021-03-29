#ifndef EXCEPTIONS_HANDLER_H
#define EXCEPTIONS_HANDLER_H

#include "pcb.h"
#include "asl.h"
#include "pandos_types.h"
#include "commons.h"
#include "scheduler.h"

void fooBar();
void SYS_handler();
int interrupt_handler();
void PassUpOrDie(int EXCEPT);

#endif /* !defined(EXCEPTIONS_HANDLER_H) */