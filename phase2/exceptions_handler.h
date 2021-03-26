#ifndef EXCEPTIONS_HANDLER_H
#define EXCEPTIONS_HANDLER_H

#include "scheduler.h"

void fooBar();
void SYS_handler();
void trap_handler();
void interrupt_handler();
void init_devices();
int check_dev_installation( int numLine, int numDev);
int check_dev_interruption( int numLine, int numDev);
int power(int base, int exp);

#endif /* !defined(EXCEPTIONS_HANDLER_H) */