#ifndef SYSSUPPORT_H
#define SYSSUPPORT_H

#include "pcb.h"
#include "asl.h"
#include "pandos_types.h"
#include "commons.h"
#include "scheduler.h"

void general_exHandle();
void syscall_exHandle();
void program_trap_exHandle();

#endif