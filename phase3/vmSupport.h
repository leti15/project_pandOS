#ifndef VMSUPPORT_H
#define VMSUPPORT_H

#include "pcb.h"
#include "asl.h"
#include "pandos_types.h"
#include "commons.h"
#include "scheduler.h"
#include "exceptions_handler.h"
#include "sysSupport.h"
#include "initProc.h"

#define INVALIDMASK 0b11111111111111111111110111111111
#define DISABLEINTERRUPTS 0b11111111111111111111111111111110
#define ENABLEINTERRUPTS 0b00000000000000000000000000000001

extern int swp_sem;
extern swap_t*  spt;

void init_spt();
swap_t* replacement_FIFO();
void update_TLB();
void update_BackingStore(swap_t* frame, int owner_process);
unsigned int ReadWrite_from_backStore(swap_t* frame, support_t* support_process, int virtualPG, int readMode);
void pager();

#endif