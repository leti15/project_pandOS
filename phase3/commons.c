#include "commons.h"

void * memcpy (void *dest, const void *src, size_t len)
{
  char *d = dest;
  const char *s = src;
  while (len--)
    *d++ = *s++;
  return dest;
}

void init_devices() {
    for (int i = 0; i < DEVARRSIZE-1; i = i + 1) { 
        devicesem[i] = 0;
    }
}

int check_dev_installation( int numLine, int numDev){
    unsigned int x, mask; 
    unsigned int* base_line = (unsigned int *)(0x1000002C);
    
    if (numLine == 4){ base_line = (unsigned int *)(0x1000002C + 0x04);}
    if (numLine == 5){ base_line = (unsigned int *)(0x1000002C + 0x08);}
    if (numLine == 6){ base_line = (unsigned int *)(0x1000002C + 0x0C);}
    if (numLine == 7){ base_line = (unsigned int *)(0x1000002C + 0X10);}
    x = *base_line;
    
    //mask = 2^numDev
    mask = 1;
    for (int i=0; i< numDev; i = i+1){ mask = mask*2; }
    if ( ((x & mask) >> numDev) > 0){ return TRUE; } 
    else { return FALSE; }
}

int check_dev_interruption( int numLine, int numDev){

    unsigned int x, mask; 
    unsigned int* base_line = (unsigned int *)(0x10000040);

    if (numLine == 4){ base_line = (unsigned int *)(0x10000040 + 0x04);}
    if (numLine == 5){ base_line = (unsigned int *)(0x10000040 + 0x08);}
    if (numLine == 6){ base_line = (unsigned int *)(0x10000040 + 0x0C);}
    if (numLine == 7){ base_line = (unsigned int *)(0x10000040 + 0X10);}
    x = *base_line;
    
    //mask = 2^numDev
    mask = 1;
    for (int i=0; i< numDev; i = i+1){ mask = mask*2; }

    if ( ((x & mask) >> numDev) > 0){ return TRUE; } 
    else { return FALSE; }
}

int check_dev_semAdd(int* semAdd){
    return (semAdd >= &devicesem[0] && semAdd <= &devicesem[DEVARRSIZE-1]);
}

/**
 * Estraggo il numero di pagina virtuale data la entryHI, se == 255 allora ....??
*/
pteEntry_t inspecteHI(int VP, support_t* support_struct){
  /*entryHi = (entryHi >> 12) && 00000000000000000011; //volevate fare l'and logico (bit a bit)?? perchè quello è solo con &, non && 
  if (entryHi == 255) 
    return 31;
  else 
    return entryHi;*/

    for (int i = 0; i< MAXPAGES; i+= 1){
      if((support_struct->sup_privatePgTbl[i].pte_entryHI >> 12) == VP)
        return (support_struct->sup_privatePgTbl[i]);
    }
    
}

void atomON(){
  state_t* state_reg = (state_t *)BIOSDATAPAGE;
  //disabilito interrupts
  setSTATUS(state_reg->status & DISABLEINTERRUPTS);
}

void atomOFF(){
  state_t* state_reg = (state_t *)BIOSDATAPAGE;
  //riabilito interrupts
  setSTATUS (state_reg->status = state_reg->status & ENABLEINTERRUPTS);
}