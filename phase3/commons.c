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
    for (int i = 0; i < DEVARRSIZE; i = i + 1) { 
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

int inspecteHI(int entryHi){
  return (entryHi >> 12) && 00000000000000000011;
}
