#ifndef COMMONS_H
#define COMMONS_H

#include <stdio.h>
#include <stdlib.h>

#include "pcb.h"
#include "asl.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "p2test.c"

#define STATE_INIT 0b00011000000000001111111100001100
#define STATE_WAIT 0b00010000000000001111111100000001

//macro to find old kernel/user mode
#define STATUSO_MODE_MASK 0b00000000000000000000000000100000
#define STATUSO_MODE_BIT 5
#define STATUSO_GET_MODE(x)   (((x) & STATUSO_MODE_MASK) >> STATUSO_MODE_BIT)

//macro to find previous kernel/user mode
#define STATUSP_MODE_MASK 0b00000000000000000000000000001000
#define STATUSP_MODE_BIT 3
#define STATUSP_GET_MODE(x)   (((x) & STATUSP_MODE_MASK) >> STATUSP_MODE_BIT)

//macro to find current kernel/user mode
#define STATUSC_MODE_MASK 0b00000000000000000000000000000010
#define STATUSC_MODE_BIT 1
#define STATUSC_GET_MODE(x)   (((x) & STATUSC_MODE_MASK) >> STATUSC_MODE_BIT)

int proc_count;
HIDDEN int softB_count;
HIDDEN pcb_PTR readyQ;
HIDDEN pcb_PTR current_proc;
HIDDEN passupvector_t PassUpVector [16];
HIDDEN passupvector_t* PUV;
/** puntatore alla coda dei semafori attivi 'semd_h' */


#endif /* !defined(COMMONS_H) */