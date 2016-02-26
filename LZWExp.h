#ifndef LZWEXP_H
#define LZWEXP_H

#include <limits.h>
#include "BitUnpacker.h"
#include "MyLib.h"

#define DEFAULT_RECYCLE_CODE 4096

#define BAD_CODE -1
#define MISSING_EOD -2
#define CORRUPT_FILE -3

/* Function pointer to method to call when a block of bytes is
 * expanded and ready for transmission or whatever.  The void * parameter
 * can point to anything, and gives hidden information to the function 
 * so that it knows what file, socket, etc. the code is going to.  "Data"
 * points to the block of expanded bytes, with numBytes bytes in the block.
 * The block is not a C string -- it may have null characters in it, and
 * is not null-terminated.
 * 
 * A call with null "data" pointer marks the end of data, and will be the
 * final call.
*/
typedef void (*DataSink)(void *, unsigned char *data, int numBytes);

/* Current state of the LZW expander. Do not access any of these
 * fields directly; they may change without warning -- use the 
 * four functions below for any LZWExp work
 *
 * NOTE: Two fields, leftOver and bitsLeft, were removed for the W15 version
 *       of this assignment due the the inclusion of BitUnpacker.
 */
typedef struct LZWExp {
   void *dict;        /* Ptr to CodeSet dictionary of codes */
   DataSink sink;     /* Data sink to send char sequences to */
   void *sinkState;   /* Unknown object to send to sink for state */
   int lastCode;      /* Last code that was created */
   int numBits;       /* Number of bits per code currently */
   int maxCode;       /* Max code that fits in numBits bits. */
   int recycleCode;   /* Code that triggers recycling of dictionary */
   BitUnpacker bitUnpacker; /* Used to uncompress data */
   int EODSeen;       /* Indicates that an EOD code has been seen */
} LZWExp;

/* Initialize a LZWExp the DataSink to which to send uncompressed symbol 
 * sequences and an anonymous state object for the DataSink.
 */
void LZWExpInit(LZWExp *exp, DataSink sink, void *sinkState, int recycleCode);

/* Breaks apart compressed data in "bits" into one or more codes and send 
 * the corresponding symbol sequences to the DataSink. Save any leftover 
 * compressed bits to combine with the bits from the next call of LZWExpDecode.
 *
 * Returns 0 on successful processing of a code (not done), 1 when EOD is
 * received (done), or BAD_CODE if a code that is not in the dictionary is
 * received (problem).
 */
int LZWExpDecode(LZWExp *exp, UInt bits);

/* Called when LZWExpDecode is done to peform error checking and/or
 * housekeeping that should be performed at the end of decoding.
 *
 * Returns 0 if all is okay, MISSING_EOD if no terminating EOD was found, or
 * CORRUPT_FILE if non-zero bits follow the EOD. 
 */
int LZWExpStop(LZWExp *exp);

/* Free all storage associated with LZWExp (not the sinkState, though,
 * which is "owned" by the caller.  Must be called even if LZWExpInit
 * returned an error.
 */
void LZWExpDestruct(LZWExp *exp);

#endif
