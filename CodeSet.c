#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "SmartAlloc.h"
#include "CodeSet.h"

typedef struct CodeEntry {
   unsigned char data;
   unsigned short size;
   struct CodeEntry *prefix;
   unsigned char *code;
   unsigned short get;
} CodeEntry;

typedef struct CodeSet {
   unsigned int size;
   CodeEntry *codes;
   int index;
} CodeSet;

void *CreateCodeSet(int numCodes) {
   CodeSet *codeSet = malloc(sizeof(CodeSet));
   
   codeSet->size = numCodes;
   codeSet->codes = malloc(sizeof(CodeEntry) * numCodes);
   codeSet->index = 0;
   return codeSet;
}
/* Add a new 1-byte code to |codeSet|, returning its index, with
 *  * the first added code having index 0.  The new code's byte is
 *   * equal to |val|.  Assume (and assert if needed) that there
 *    * is room in the |codeSet| for a new code. */
int NewCode(void *codeSet, char val) {
   CodeSet *s = codeSet;
   CodeEntry *ce = s->codes;
   CodeEntry *ndx = ce + s->index;
   
   ndx->data = val;
   ndx->size = 1;
   ndx->get = 0;
   ndx->prefix = 0;
   ndx->code = NULL;
   return s->index++;
}

int ExtendCode(void *codeSet, int oldCode) {
   CodeSet *s = codeSet;
   CodeEntry *ce = s->codes;
   CodeEntry *ndx = ce + s->index;
   
   ndx->size = (ce + oldCode)->size + 1;
   ndx->data = 0;
   ndx->get = 0;
   ndx->prefix = ce + oldCode;
   ndx->code = NULL;
   return s->index++;
}

void SetSuffix(void *codeSet, int code, char suffix) {
   CodeEntry *ce = ((CodeSet *)codeSet)->codes + code;
   
   ce->data = suffix;
   if (ce->code) {
      *(ce->code + ce->size - 1) = suffix;
   }
}

Code GetCode(void *codeSet, int code) {
   int depth;
   CodeEntry *ce = ((CodeSet *)codeSet)->codes + code;
   CodeEntry *temp = ce;
   Code c;
   
   c.size = ce->size;
   depth = c.size;
   if (temp->get) {
      c.data = temp->code;
   }
   else {
      c.data = malloc(sizeof(char) * c.size);
      while (depth--) {
         if (temp->code) {
            memcpy(c.data, temp->code, temp->size);
            depth = 0;
         }
         else {
            *(c.data + depth) = temp->data;
            temp = temp->prefix;
         }
      }
      ce->code = c.data;
   }
   ce->get++;
   return c;
}


void FreeCode(void *codeSet, int code) {
   CodeEntry *ce = ((CodeSet *)codeSet)->codes + code;
   
   if (ce->get > 1) {
      ce->get--;
   }
   else if (ce->get) {
      free(ce->code);
      ce->code = NULL;
      ce->get--;
   }
}

/* Free all dynamic storage associated with |codeSet| */
void DestroyCodeSet(void *codeSet) {
   CodeSet *s = codeSet;
   CodeEntry *ce = s->codes;
   int ndx = 0;
   
   while (ndx++ < s->index) {
      if (ce->code) {
         free(ce->code);
      }
      ce++;
   }
   free(s->codes);
   free(s);
}
