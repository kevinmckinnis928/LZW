#include "CodeSet.h"
#include <math.h>
#include "LZWExp.h"

#define START_SIZE 256
#define START_BITS 9

void CreateDict(void *codeSet) {
   int i;
   
   for (i = 0; i < START_SIZE; i++) {
      NewCode(codeSet, (char) i);
   }
   NewCode(codeSet, 0);
}

void LZWExpInit(LZWExp *exp, DataSink sink, void *sinkState, int recycleCode)
{
   BitUnpacker bu;
   
   exp->dict = CreateCodeSet(recycleCode);
   exp->sink = sink;
   exp->sinkState = sinkState;
   exp->recycleCode = recycleCode;
   buInit(&bu);
   exp->bitUnpacker = bu;
   exp->numBits = START_BITS;
   exp->lastCode = START_SIZE;
   exp->maxCode = (1 << exp->numBits) - 1;
   exp->EODSeen = 0;
   CreateDict(exp->dict);
}

/* Breaks apart compressed data in "bits" into one or more codes and send 
 * the corresponding symbol sequences to the DataSink. Save any leftover 
 * compressed bits to combine with the bits from the next call of LZWExpDecode.
 *
 * q
 * Returns 0 on successful processing of a code (not done), 1 when EOD is
 * received (done), or BAD_CODE if a code that is not in the dictionary is
 * received (problem).
 */
int LZWExpDecode(LZWExp *exp, UInt bits) {
   UInt data = 0;
   Code code;
   
   buTakeData(&(exp->bitUnpacker), bits);
   while (unpack(&exp->bitUnpacker, exp->numBits, &data)) {
      if (data == START_SIZE) {
         exp->EODSeen = 1;
         return exp->EODSeen;
      }
      else {
         if (data > exp->lastCode) {
            return BAD_CODE;
         }
         code = GetCode(exp->dict, data);
         if (exp->lastCode != START_SIZE) {
            SetSuffix(exp->dict, exp->lastCode, *(code.data));
         }
         exp->sink(exp->sinkState, code.data, code.size);
         FreeCode(exp->dict, data);
         exp->lastCode++;
         if (exp->lastCode > exp->recycleCode - 1) {
            LZWExpDestruct(exp);
            exp->dict = CreateCodeSet(exp->recycleCode);
            CreateDict(exp->dict);
            exp->numBits = START_BITS;
            exp->lastCode = START_SIZE;
            exp->maxCode = (1 << exp->numBits) - 1;
         }
         else {
            ExtendCode(exp->dict, data);
            if (exp->lastCode > exp->maxCode) {
               exp->numBits++;
               exp->maxCode = (1 << exp->numBits) - 1;
            }
         }
      }
   }
   return exp->EODSeen;
}

/* Called when LZWExpDecode is done to peform error checking and/or
 * housekeeping that should be performed at the end of decoding.
 *
 * Returns 0 if all is okay, MISSING_EOD if no terminating EOD was found, or
 * CORRUPT_FILE if non-zero bits follow the EOD. 
 */
int LZWExpStop(LZWExp *exp) {
   UInt remaining;
   
   if (!exp->EODSeen) {
      return MISSING_EOD;
   }
   if (exp->bitUnpacker.bitsLeft > 0) {
      remaining = 0;
      unpack(&(exp->bitUnpacker), exp->bitUnpacker.bitsLeft, &remaining);
      if (remaining) {
         return CORRUPT_FILE;
      }
   }
   return 0;
}

/* Free all storage associated with LZWExp (not the sinkState, though,
 * which is "owned" by the caller.  Must be called even if LZWExpInit
 * returned an error.
 */
void LZWExpDestruct(LZWExp *exp) {
   DestroyCodeSet(exp->dict);
}


