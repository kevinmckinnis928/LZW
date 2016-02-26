#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UINT_SIZE 32
#define UINT_MASK 0xFFFFFFFF

typedef unsigned int UInt;

typedef struct {
   UInt curData;
   UInt nextData;
   int bitsLeft;
   int validNext;
} BitUnpacker;

void buInit(BitUnpacker *bu) {
   bu->bitsLeft = 0;
   bu->validNext = 0;
   bu->curData = 0;
   bu->nextData = 0;
}

void buTakeData(BitUnpacker *bu, UInt next) {
   bu->nextData = next;
   bu->validNext = 1;
}

int unpack(BitUnpacker *bu, int size, UInt *ret) {
   if (!bu->bitsLeft && bu->validNext) {
      bu->curData = bu->nextData;
      bu->bitsLeft = UINT_SIZE;
      bu->validNext = 0;
   }
   if (bu->bitsLeft >= size) {
      bu->bitsLeft -= size;
      *ret = bu->curData >> bu->bitsLeft;
      if(size != UINT_SIZE) {
         *ret = *ret & ((1<<size)-1);
      }
      return 1;
   }
   else if (bu->bitsLeft < size && bu->validNext) {
      size -= bu->bitsLeft;
      *ret = (bu->curData & ((1<<bu->bitsLeft)-1)) << size;
      bu->curData = bu->nextData;
      bu->validNext = 0;
      bu->bitsLeft = UINT_SIZE - size;
      *ret = *ret | (bu->curData >> bu->bitsLeft & ((1<<size)-1));
      return 1;
   }
   return 0;
}
