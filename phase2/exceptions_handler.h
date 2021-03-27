#ifndef EXCEPTIONS_HANDLER_H
#define EXCEPTIONS_HANDLER_H

#include "scheduler.h"

void fooBar();
void SYS_handler();
void trap_handler();
void interrupt_handler(unsigned int current_causeCode, int exCode);

#endif /* !defined(EXCEPTIONS_HANDLER_H) */