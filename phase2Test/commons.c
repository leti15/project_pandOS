#include "commons.h"

void breakPoint(){}
void breakPoint2(){}
void breakPoint3(){}

void * memcpy (void *dest, const void *src, size_t len)
{
  char *d = dest;
  const char *s = src;
  while (len--)
    *d++ = *s++;
  return dest;
}

void remove_from_arrayDev( int* semAdd){
    for (int i = 0; i < DEVARRSIZE; i = i+1){
        if (device[i]->s_semAdd == semAdd){
            device[i] = NULL;
            break;
        }
    }
}

void init_devices() {
    for (int i = 0; i < DEVARRSIZE; i = i + 1) { device[i] = NULL; }
}

int check_dev_installation( int numLine, int numDev){

    unsigned int x, mask; 
    unsigned int* base_line = (unsigned int *) 0x1000002C;
    if (numLine == 4){ base_line = base_line + 0x04;}
    if (numLine == 5){ base_line = base_line + 0x08;}
    if (numLine == 6){ base_line = base_line + 0x0C;}
    if (numLine == 7){ base_line = base_line + 0X10;}
    x = *base_line;
    
    //mask = 2^numDev
    mask = 1;
    for (int i=0; i< numDev; i = i+1){ mask = mask*2; }

    if ( ((x & mask) >> numDev) > 0){ return TRUE; } 
    else { return FALSE; }
}

int check_dev_interruption( int numLine, int numDev){

    unsigned int x, mask; 
    unsigned int* base_line = (unsigned int*) 0x10000040;
    if (numLine == 4){ base_line = base_line + 0x04;}
    if (numLine == 5){ base_line = base_line + 0x08;}
    if (numLine == 6){ base_line = base_line + 0x0C;}
    if (numLine == 7){ base_line = base_line + 0X10;}
    x = *base_line;
    
    //mask = 2^numDev
    mask = 1;
    for (int i=0; i< numDev; i = i+1){ mask = mask*2; }

    if ( ((x & mask) >> numDev) > 0){ return TRUE; } 
    else { return FALSE; }
}

int check_dev_semAdd(int* semAdd){
    for (int i=0; i<DEVARRSIZE; i = i+1){
        if (device[i]->s_semAdd == semAdd){
            return TRUE;
        }
    }
    return FALSE;
}

int find_dev_index(int* semAdd){
    for (int i=0; i<DEVARRSIZE; i = i+1){
        if (device[i]->s_semAdd == semAdd){
            return i;
        }
    }
}