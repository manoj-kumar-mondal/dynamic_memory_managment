
#include "heap.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/******************** Constants / Typedefs ********************/
typedef uint16_t MemPoolIndex_t;

#pragma pack(1) // Prgma pack starts
typedef struct {
  MemPoolIndex_t Index;
  MemPoolIndex_t Size;
}MemMetaData_t;

typedef struct {
  MemPoolIndex_t MetaDataIndex;
  MemPoolIndex_t AvailableMem;
}MemPoolData_t;
#pragma pack() // pragma pack ends

#define META_DATA_SIZE    (sizeof(MemMetaData_t))
#define TAB               ("    ")

/******************** Variable Declaration ********************/
static uint8_t MemoryPool[CONFIG_HEAP_SIZE];
static const uint8_t *const pMemPoolStart = &MemoryPool[0];
static const uint8_t *const pMemPoolEnd = (pMemPoolStart + CONFIG_HEAP_SIZE);
static MemPoolData_t MemPoolData = {
  .MetaDataIndex = sizeof(MemoryPool),
  .AvailableMem = sizeof(MemoryPool)
};

/**************** Static Function Declaration ****************/
static int CompareByIndex(const void *item1, const void *item2);
static uint8_t *MemAlloc(MemPoolIndex_t Size);
static uint8_t *AllocateMemory(MemPoolIndex_t Size);
static MemPoolIndex_t IfValidAddrThenDelete(uint8_t *pAddr);
static MemPoolIndex_t DeallocateMemory(uint8_t *pAddr);

/****************** Debug Fuctions ******************/
#ifdef DEBUG_MODE

void PrintMemoryPool(void) {
  printf("\nMemory Pool:");

  printf("%s%s%s", TAB, TAB, TAB);
  for(uint8_t i = 0 ; i <= 0xf ; i++)
    printf("%X%s", i, TAB);

  for (MemPoolIndex_t Index = 0 ; Index < CONFIG_HEAP_SIZE ; Index++) {
    if (!(Index % 16)) {
      printf("\n%s%p%s", TAB, &MemoryPool[Index], TAB);
    }
    printf("0x%02x ", MemoryPool[Index]);
  }
  printf("\n\n");
}

void MemPoolInfo(void) {
  printf("\nMemPoolInfo:\n");
  printf("%sAvailableMem: %d, ", TAB, MemPoolData.AvailableMem);
  printf("MetaDataIndex: %d\n\n", MemPoolData.MetaDataIndex);
}

#endif // DEBUG_MODE

/******************** Function Defintions ********************/

/**
 * @brief Allocate memory from the heap
 * @param Size: Size of memory to be allocated
 * @return void*: Pointer to the allocated memory
 */
void *my_malloc(size_t Size) {
  return ((void*)AllocateMemory(Size));
}

/**
 * @brief Free the allocated memory
 * @param pAddr: Pointer to the memory to be freed
 * @return uint8_t: Status of the operation
 */
uint8_t my_free(void *pAddr) {
  uint8_t Status = DeallocateMemory((uint8_t*)pAddr);
  return 0;
}

/******************* Heap Manager Functions *******************/

/**
 * @brief Compare function for qsort
 * @param item1: Pointer to the first item
 * @param item2: Pointer to the second item
 * @return int: Comparison result of the two items
 */
static int CompareByIndex(const void *item1, const void *item2) {
  const MemMetaData_t *pItem1 = (const MemMetaData_t *)item1;
  const MemMetaData_t *pItem2 = (const MemMetaData_t *)item2;
  return (pItem1->Index - pItem2->Index);
}

/**
 * @brief Allocate memory from the heap
 * @param Size: Size of memory to be allocated
 * @return uint8_t*: Pointer to the allocated memory
 */
static uint8_t *MemAlloc(MemPoolIndex_t Size) {
  printf("Allocating Index ... \n");
  const uint16_t TotalAllocatedIndex = ((CONFIG_HEAP_SIZE - MemPoolData.MetaDataIndex) / META_DATA_SIZE);
  MemMetaData_t AllocMetaInfo[TotalAllocatedIndex];
  MemPoolIndex_t ArrIndex = 0;
  uint8_t *pAllocAddr = NULL;
  uint8_t *pTemp = (uint8_t*)&MemoryPool[MemPoolData.MetaDataIndex];

  while (pTemp < pMemPoolEnd) {
    memcpy((void*)&AllocMetaInfo[ArrIndex++], (void*)pTemp, META_DATA_SIZE);
    pTemp += META_DATA_SIZE;
  }

  if (TotalAllocatedIndex > 1) {
    // Sort the array according to index
    qsort((void*)AllocMetaInfo, TotalAllocatedIndex, META_DATA_SIZE, CompareByIndex);
  }

  MemPoolIndex_t PotentialIndex = 0;

  for (ArrIndex = 0 ; ArrIndex < TotalAllocatedIndex ; ArrIndex++) {
    MemPoolIndex_t AvailableSize = AllocMetaInfo[ArrIndex].Index - PotentialIndex;
    if (AvailableSize >= Size) {
      break;
    }
    PotentialIndex = (AllocMetaInfo[ArrIndex].Index + AllocMetaInfo[ArrIndex].Size);
  }

  pAllocAddr = (uint8_t*)&MemoryPool[PotentialIndex];
  memset(pAllocAddr, 0xFF, Size);
  pAllocAddr[Size-1] = '\0';
  MemPoolData.AvailableMem -= Size;
  printf("Allocation Done at Index: %d\n", PotentialIndex);
  return pAllocAddr;  
}

/**
 * @brief Allocate memory from the heap
 * @param Size: Size of memory to be allocated
 * @return uint8_t*: Pointer to the allocated memory
 */
static uint8_t *AllocateMemory(MemPoolIndex_t Size) {
  if ((CONFIG_HEAP_SIZE < Size) || \
      ((MemPoolData.AvailableMem - META_DATA_SIZE) < Size)) {
      printf("Memory allocation failed!!!, Avail: %d, Ask: %d\n", \
          (MemPoolData.AvailableMem - META_DATA_SIZE - 1), Size);
      return NULL;
    }
  
  uint8_t *pAllocateMem = MemAlloc(++Size); // Add one more byte for memory fragmentation

  if (pAllocateMem) {
    if ((pMemPoolStart <= pAllocateMem) && (pMemPoolEnd > pAllocateMem)) {
      const MemPoolIndex_t AllocatedMemIndex = (MemPoolIndex_t)(pAllocateMem - pMemPoolStart);
      MemPoolIndex_t MetaDataIndex = MemPoolData.MetaDataIndex;
      MemMetaData_t MemMetaData = {
        .Index = AllocatedMemIndex,
        .Size = Size
      };
      
      MetaDataIndex -= META_DATA_SIZE;

      uint8_t *pWriteAddr = (uint8_t*)&MemoryPool[MetaDataIndex];
      memcpy(pWriteAddr, (void*)&MemMetaData, META_DATA_SIZE);
      MemPoolData.MetaDataIndex = MetaDataIndex;
      MemPoolData.AvailableMem -= META_DATA_SIZE;

    } else {
      pAllocateMem = NULL;
    }
  }

  return pAllocateMem;
}

/**
 * @brief Check if the address is valid and then delete the memory
 * @param pAddr: Pointer to the memory to be deleted
 * @return MemPoolIndex_t: Size of the memory deleted
 */
static MemPoolIndex_t IfValidAddrThenDelete(uint8_t *pAddr) {
  const uint8_t *pMetaDataIndexAddr = (uint8_t*)&MemoryPool[MemPoolData.MetaDataIndex];
  uint8_t *pTemp = (uint8_t*)(pMemPoolEnd - META_DATA_SIZE);
  MemPoolIndex_t Size = 0;

  while(pTemp >= pMetaDataIndexAddr) {
    MemPoolIndex_t *pMemIndex = (MemPoolIndex_t*)pTemp;
    if (pAddr == (uint8_t*)&MemoryPool[*pMemIndex]) {
      Size = *(++pMemIndex);
      printf("SizeL %x\n", Size);
      break;
    }
    pTemp -= META_DATA_SIZE;
  }

  if(Size) {
    if (pMetaDataIndexAddr != pTemp) {
      memcpy(pTemp, pMetaDataIndexAddr, META_DATA_SIZE);
    }
    memset((void*)pMetaDataIndexAddr, 0x00, META_DATA_SIZE);
    MemPoolData.MetaDataIndex += META_DATA_SIZE;
    MemPoolData.AvailableMem += META_DATA_SIZE;
  }
  return Size;
}

/**
 * @brief Deallocate the memory
 * @param pAddr: Pointer to the memory to be deallocated
 * @return MemPoolIndex_t: Size of the memory deallocated
 */
static MemPoolIndex_t DeallocateMemory(uint8_t *pAddr) {
  MemPoolIndex_t Size = 0;

  if ((pMemPoolStart <= pAddr) && (pMemPoolEnd > pAddr)) {
    Size = IfValidAddrThenDelete(pAddr);
    
    if (Size) {
      memset(pAddr, 0x00, Size);
      MemPoolData.AvailableMem += Size;
    }
  }
  return Size;
}