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

extern int swp_sem;
extern swap_t*  spt;

void init_spt();
swap_t* replacement_FIFO();
void update_TLB();
void update_BackingStore();
pteEntry_t read_from_backStore(int pageNO);
void pager();

#endif