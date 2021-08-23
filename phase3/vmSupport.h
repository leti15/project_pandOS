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

void uTLB_RefillHandler();
void init_spt();
int replacement_FIFO();
void update_TLB();
unsigned int ReadWrite_from_backStore(int processID, int blocknumber, unsigned int to_write_or_read, int readMode);
void pager();

#endif