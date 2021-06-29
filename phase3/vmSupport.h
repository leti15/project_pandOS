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

extern int swp_sem;
extern swap_t*  spt;

void init_spt();
swap_t* replacement_FIFO();
void update_TLB();
void update_BackingStore(swap_t* frame, int owner_process);
unsigned int ReadWrite_from_backStore(swap_t* frame, pteEntry_t* page_table, int processID, int virtualPG, unsigned int PFN, int readMode);
void pager();

#endif