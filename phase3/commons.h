#ifndef COMMONS_H
#define COMMONS_H
//#include <stddef.h>
#include "pandos_types.h"
#include <umps3/umps/libumps.h>

#ifndef __SIZE_TYPE__
#define __SIZE_TYPE__ long unsigned int
#endif
typedef __SIZE_TYPE__ size_t;

#define DEFAULT_RESOURCE 0

#define STATE_INIT 0b00011000000000001111111100000100
#define STATE_WAIT 0b00010000000000001111111100000001

#define DEVARRSIZE 49 //grandezza array dei semafori dei device

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

#define EXC_MASK 0b00000000000000000000000001111100
#define CAUSE_GET_EXCCODE(x)    (((x) & EXC_MASK) >> 2)
#define SWAP_POOL_START 0x20020000

#define INVALIDbitV 0b11111111111111111111110111111111
#define VALIDbitV 0b00000000000000000000001000000000
#define DISABLEINTERRUPTS 0b11111111111111111111111111111110
#define ENABLEINTERRUPTS 0b00000000000000000000000000000001
#define RECVSTATUSBIT 0b00000000000000000000000011111111
#define CHARCVED 5 

#define VPNBASE 0x80000000
#define STACKPG 0xBFFFF000
#define Nproc 8

extern int devicesem[DEVARRSIZE];
extern int count_time;
extern int proc_count;
extern int softB_count;
extern pcb_PTR readyQ;
extern pcb_PTR current_proc;
extern passupvector_t* PUV;
extern int devRegSem[8];


extern void bp();
extern void bp1();
extern void bp2();
extern void bp3();
extern void b4();
extern void b5();
extern void b6();
extern void boo();

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
extern void sys_terminate();
extern void sys_8();
extern void sys_9( support_t* support_except);
extern void sys_10(support_t* support_except);
extern void sys_11(support_t* support_except);
extern void sys_12(support_t* support_except);
extern void sys_13(support_t* support_except);
extern void DeleteProcPages(int asid);

void * memcpy (void *dest, const void *src, size_t len);
void init_devices();
void init_spt();
int check_dev_installation( int numLine, int numDev);
int check_dev_interruption( int numLine, int numDev);
int check_dev_semAdd(int* semAdd);
pteEntry_t inspecteHI(int VP, support_t* support_struct);
void atomON();
void atomOFF();

#endif /* !defined(COMMONS_H) */
