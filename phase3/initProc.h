#ifndef INITPROC_H
#define INITPROC_H

#include "pcb.h"
#include "asl.h"
#include "pandos_types.h"
#include "commons.h"
#include "scheduler.h"
#include "vmSupport.h"
#include "sysSupport.h"

void test();
void initPGTBL(pteEntry_t* pgtable, int asid);
void init_supLevSem();
#endif