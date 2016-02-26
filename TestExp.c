#include "MyLib.h"
#include "LZWExp.h"
#include "CodeSet.h"
#include "SmartAlloc.h"
#include <assert.h>

void Sink(void *state, unsigned char *data, int numBytes) {
   while (numBytes--) {
      printf("%c", *(data++));
   }
}

int main(int argc, char **argv) {
   LZWExp exp;
   int rFlag = 0;
   UInt cur = 0;
   int keepGoing = 0;
   char **temp = argv;
   int val, state;
    
   while (*++temp && !rFlag) {
      if (!strcmp(*temp, "-R")) {
         rFlag = 1;
      }
   }
   if (!rFlag) {
      LZWExpInit(&exp, Sink, 0, DEFAULT_RECYCLE_CODE);
   }
   
   else {
      val = atoi(*temp);
      LZWExpInit(&exp, Sink, 0, val);
   }
   
   while (keepGoing == 0 && EOF != scanf("%x", &cur)) {
      keepGoing = LZWExpDecode(&exp, cur);
   }
   if (keepGoing == BAD_CODE) {
      printf("Bad code\n");
   }
   
   else {
      state = LZWExpStop(&exp);
      if (state == MISSING_EOD) {
         printf("Missing EOD\n");
      }
      if (state == CORRUPT_FILE) {
         printf("Corrupt file\n");
      }
      if (scanf("%x", &cur) != EOF) {
         printf("Corrupt file\n");
      }
   }
   LZWExpDestruct(&exp);
   assert(report_space() == 0);
   return 0;
}
