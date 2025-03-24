#include "heap.h"
#include <stdio.h>

int main() {
  #if 0
  // MemPoolInfo();
  uint8_t *pMemAllocs[] = {
    my_malloc(4),
    my_malloc(8),
    my_malloc(6),
  };
  // // PrintMemoryPool();
  // // MemPoolInfo();
  my_free((void*)pMemAllocs[1]);
  
  uint8_t *pTemp =  my_malloc(10), *pTemp2;
  my_free((void*)pMemAllocs[0]);
  pTemp =  my_malloc(7);
  pTemp =  my_malloc(4);
  my_free((void*)pMemAllocs[2]);
  pTemp2 =  my_malloc(6);
  my_free(pTemp);
  pTemp2 =  my_malloc(4);
  PrintMemoryPool();
  MemPoolInfo();
  // printf("Main(): Allocted Mem at %p\n", pMem);
  #endif
  return 0;
}