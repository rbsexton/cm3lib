// 
// Lockless Ringbuffer routines
// Copyright(C) 2012-2016 Robert Sexton
//
// Basic Model - Struct with external storage
// - Lockless - No Irq protection required.
// - Dynamically created so that the same code maintains multiple buffers.
// - Counted contents so you can put in a big chunk.
// - Requires power of two sizing.
//
// Ringbuffer, v2.   Make things more robust to better live in a world
// without locking.   Two pointers, reader and writer.  
// They both get used modulo the pointer size.   Equality means empty.
// If the writer pre-empts the reader, it'll wrongly assume fullness.  No problem.
// If the reader pre-empts the writer, it might wrongly assume empty.  No Problem.
//  
// Do not permit the read and write pointer to ever wrap.   Return -1 rather than
// dropping characters.   The user must decide what to do in that case.
// 

#include <stdint.h>
#include "ringbuffer.h"

/// Initialization call.  
/// @parameters
/// a pointer to an ringbuffer structure
/// a pointer to the buffer that will hold the data
void ringbuffer_init(RINGBUF* rb, uint8_t* buf, int size) {
	rb->iRead=0;   // Read index - points to the next character ready for reading
	rb->iWrite=0;  // Write index - Points to next free character
	rb->Dropped=0; // Dropped character count
	rb->Buf = buf; // The user provides a pointer to the storage area.
	rb->BufSize = size;
	rb->BufMask = size - 1;
	}

/// The number of characters 
// There used to be a not-empty check, but it turns out that
// ringbuffer_used is a bit faster after compilation is over.
// Used is just a subtract, so the conditional is free.	
uint32_t ringbuffer_used(RINGBUF* rb) {
	return(rb->iWrite - rb->iRead);
	}

/// The ring buffers are not allowed to fill up.
/// That means that the maximal room is BufSize - 1 = BufMask 
uint32_t ringbuffer_free(RINGBUF* rb) {
	return(rb->BufMask - ringbuffer_used(rb));
	}

/// Add a character to the ringbuffer, and return the 
/// number of characters free in the ring buffer.
/// 
/// If incrementing the write counter makes it equal to 
/// the read counter, that means that we're full.   Reject it and
/// return -1 so that the producer can retry.
int32_t ringbuffer_addchar(RINGBUF* rb, uint8_t c) {
	uint32_t iWritePend = rb->iWrite + 1; 

	// Don't clobber the existing data.
	if ( (iWritePend & rb->BufMask) != (rb->iRead & rb->BufMask) ) {
		rb->Buf[rb->iWrite & rb->BufMask] = c; 
		rb->iWrite = iWritePend;
		return(ringbuffer_free(rb));
		}
	else { // Back-pressure.
		rb->Dropped++;
		return(-1);
		}
	}

/// Pull a character out of the ringbuffer
/// Return -1 if there is nothing in there. 
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

/// Reset the two pointers to prevent wrap.
/// critial locking must come from outside.
/// Ideally via a pended low priority interrupt.
void ringbuffer_reset(RINGBUF* rb) {
	if ( rb->iRead  < RB_COUNT_OVERFLOW ||
	     rb->iWrite < RB_COUNT_OVERFLOW ) return;
	     
	rb->iRead -= RB_COUNT_OVERFLOW;
	rb->iWrite -= RB_COUNT_OVERFLOW;
	rb->ResetCount++;
	}

/// -----------------------------------------------------------
/// Bulk operations.
/// There are some conditions where we want to bypass the
/// character-at-a-time aspect of things and do a bulk copy
/// Networking and USB are the primary cases.
/// So there will be one or two bulk operatons - one to collect
/// all data up to the end of the storage area, and one to
/// collect everything from the beginning.
/// -----------------------------------------------------------

// Pretty easy.   Just get the pointer to the next byte.
uint8_t *ringbuffer_getbulkpointer(RINGBUF* rb) {
	if ( ringbuffer_used(rb) ) { 
		return(&(rb->Buf[rb->iRead & rb->BufMask]));
		}
	else return(0);
	}

/// How many bytes are left?
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
	
/// This one needs to run on trust.	
void ringbuffer_bulkremove(RINGBUF* rb, int count) {
	rb->iRead += count;
	}
