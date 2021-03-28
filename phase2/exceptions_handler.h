#ifndef EXCEPTIONS_HANDLER_H
#define EXCEPTIONS_HANDLER_H

#include "scheduler.h"

void fooBar();
void SYS_handler();
int interrupt_handler();
void PassUpOrDie(int EXCEPT);

#endif /* !defined(EXCEPTIONS_HANDLER_H) */