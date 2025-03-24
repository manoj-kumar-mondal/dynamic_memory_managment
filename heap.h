
#ifndef _MY_HEAP_MANAGER_H_
#define _MY_HEAP_MANAGER_H_

#include <stdint.h>
#include <stddef.h>

#define CONFIG_HEAP_SIZE    0x10UL
#define DEBUG_MODE

void *my_malloc(size_t size);
uint8_t my_free(void *pAddress);

#ifdef DEBUG_MODE
  void PrintMemoryPool(void);
  void MemPoolInfo(void);
#endif // DEBUG_MODE

#endif // _MY_HEAP_MANAGER_H_