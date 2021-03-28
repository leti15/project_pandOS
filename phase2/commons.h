#ifndef COMMONS_H
#define COMMONS_H

#include "pcb.h"
#include "asl.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "p2test.c"

#define STATE_INIT 0b00011000000000001111111100001100
#define STATE_WAIT 0b00010000000000001111111100000001

#define DEVARRSIZE 50 //grandezza array dei semafori dei device

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

//macro to find if a device is installed or not
#define MASKySHIFTz(x, y, z)    (((x) & y) >> z)

//prende in input il numero della line a e del device, torna l'indirizzo del device register
#define GET_devAddrBase(LINE, DEV)  (0x10000054 + ((LINE - 3) * 0x80) + (DEV * 0x10))

extern int proc_count;
extern int softB_count;
extern pcb_PTR readyQ;
extern pcb_PTR current_proc;
extern passupvector_t* PUV;
/** puntatore alla coda dei semafori attivi 'semd_h' */

//device semaphores
/**
 * 0-7: DEVICE LINEA 3
 * 8-15:DEVICE LINEA 4
 * 16-23: DEVICE LINEA 5
 * 24-31: DEVICE LINEA 6
 * 32-47: DEVICE LINEA 7 (TERMINALI)
 * 48: DEVICE INTERVAL TIMER
 * 49: DEVICE PLT
*/
extern semd_t* device[DEVARRSIZE]; 

extern void fooBar();
extern void test(); 

void remove_from_arrayDev( int* semAdd);
void init_devices();
int check_dev_installation( int numLine, int numDev);
int check_dev_interruption( int numLine, int numDev);
int check_dev_semAdd(int* semAdd);

#endif /* !defined(COMMONS_H) */