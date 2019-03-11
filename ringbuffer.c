/**
@file ringbuffer.c
@brief  Lockless Ringbuffer routines
\copyright Copyright(C) 2012-2016 Robert Sexton
@details 
Basic Model - Struct with external storage
- Lockless - No Irq protection required.
- Dynamically created so that the same code maintains multiple buffers.
- Counted contents so you can put in a big chunk.
- Requires power of two sizing.

Two pointers, reader and writer.  
They both get used modulo the pointer size.   Equality means empty.
- If the writer pre-empts the reader, it'll wrongly assume fullness.  No problem.
- If the reader pre-empts the writer, it might wrongly assume empty.  No Problem.

The read and write pointer are not allowed to wrap. 

Return -1 rather than dropping characters on a failed add.
The user must decide what to do in that case.
*/
//

#include <stdint.h>
#include "ringbuffer.h"

/// @brief Initialization call.  
/// @param rb pointer to an ringbuffer structure
/// @param buf pointer to the buffer that will hold the data
/// @param size power of two size
void ringbuffer_init(RINGBUF* rb, uint8_t* buf, int size) {
	rb->iRead=0;   // Read index - points to the next character ready for reading
	rb->iWrite=0;  // Write index - Points to next free character
	rb->Dropped=0; // Dropped character count
	rb->Buf = buf; // The user provides a pointer to the storage area.
	rb->BufSize = size;
	rb->BufMask = size - 1;
	}

/// @brief The number of characters in the buffer
// There used to be a not-empty check, but it turns out that
// ringbuffer_used is a bit faster after compilation is over.
// Used is just a subtract, so the conditional is free.	
/// @return The number of bytes currently stored on the buffer
/// @param rb pointer to a ringbuffer structure
uint32_t ringbuffer_used(RINGBUF* rb) {
	return(rb->iWrite - rb->iRead);
	}

/// @brief How many characters before it fills up?
/// @detail The ring buffers are not allowed to fill up.
/// @detail That means that the maximal room is BufSize - 1 = BufMask 
/// @param rb pointer to a ringbuffer structure
uint32_t ringbuffer_free(RINGBUF* rb) {
	return(rb->BufSize - ringbuffer_used(rb));
	}

/// @brief Add a character to the ringbuffer
/// @return number of remaining spaces in the ringbuffer after the write
/// @return return -1 on write failure.
/// @param rb pointer to a ringbuffer structure
/// @param c character to add to the ringbuffer
// Its not enough to check for wrap, we have to use math to get it right.
// If full, return -1 so that the producer can retry.
//
int32_t ringbuffer_addchar(RINGBUF* rb, uint8_t c) {
	uint32_t iWritePend = rb->iWrite + 1; 

	// Don't clobber the existing data.
	if ( (iWritePend - rb->iRead) <= rb->BufSize ) {
		rb->Buf[rb->iWrite & rb->BufMask] = c; 
		rb->iWrite = iWritePend;
		return(ringbuffer_free(rb));
		}
	else { // Back-pressure.
		rb->Dropped++;
		return(-1);
		}
	}

/// @brief Pull a character out of the ringbuffer
/// @return the next character in the ringbuffer, or -1 for empty. 
/// @param rb pointer to a ringbuffer structure
int ringbuffer_getchar(RINGBUF* rb) {
	uint8_t c;
	if ( ringbuffer_used(rb) ) { 
		// Get the char, then advance the pointer
		c = rb->Buf[rb->iRead & rb->BufMask];
		rb->iRead += 1;
		return(c);
		}
	else return(-1);
	}

/// @brief Reset the two pointers to prevent wrap.
/// @detail Not thread safe.
/// @param rb pointer to a ringbuffer structure
void ringbuffer_reset(RINGBUF* rb) {
	if ( rb->iRead  < RB_COUNT_OVERFLOW ||
	     rb->iWrite < RB_COUNT_OVERFLOW ) return;
	     
	rb->iRead -= RB_COUNT_OVERFLOW;
	rb->iWrite -= RB_COUNT_OVERFLOW;
	rb->ResetCount++;
	}

// -----------------------------------------------------------
// Bulk operations.
// There are some conditions where we want to bypass the
// character-at-a-time aspect of things and do a bulk copy
// Networking and USB are the primary cases.
// So there will be one or two bulk operatons - one to collect
// all data up to the end of the storage area, and one to
// collect everything from the beginning.
// -----------------------------------------------------------

/// @brief Get a pointer for a bulk removal.
/// @return pointer ro the largest available contiguous block of characters.
/// @param rb pointer to a ringbuffer structure
// Pretty easy.   Just get the pointer to the next byte.
uint8_t *ringbuffer_getbulkpointer(RINGBUF* rb) {
	if ( ringbuffer_used(rb) ) { 
		return(&(rb->Buf[rb->iRead & rb->BufMask]));
		}
	else return(0);
	}

/// @return length of the largest available contiguous block of characters.
/// @param rb pointer to a ringbuffer structure
int32_t ringbuffer_getbulkcount(RINGBUF* rb) {
	int used = ringbuffer_used(rb);
	if ( used ) {
		// If it points to 0xff we have 0x100 - 0xff = 1 byte in the queue
		int32_t max = rb->BufSize-(rb->iRead & rb->BufMask);
		
		if ( max >= used) return(used);
		else return(max);
		}
	else return(used);
	}

/// @brief After a bulk read, we need to do a bulk remove	
/// @param rb pointer to a ringbuffer structure
/// @param count How many characters to remove.
void ringbuffer_bulkremove(RINGBUF* rb, int count) {
	rb->iRead += count;
	}
