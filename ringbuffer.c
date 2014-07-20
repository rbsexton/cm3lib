// 
// Ringbuffer routines tailored to my particular needs.
// Copyright(C) 2012 Robert Sexton
//
// Basic Model - Struct with external storage
// - No interrupt protection.  Thats externally supplied.
// - Dynamically created so that the same code maintains multiple buffers.
// - Counted contents so you can put in a big chunk.
// - Requires power of two sizing.
//
// Ringbuffer, v2.   Make things more robust to better live in a world
// without locking.   Two pointers, reader and writer.  
// They both get used modulo the pointer size.   Equality means empty.
// If the writer pre-empts the reader, it'll wrongly assume fullness.  No problem.
// If the reader pre-empts the writer, it mighy wrongly assume empty.  No Problem.
//  
// Do not permit the read and write pointer to ever wrap.   We're going to drop
// characters when things overflow.   The cost of adjusting pointers to handle that
// case is just too high.
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
int32_t ringbuffer_free(RINGBUF* rb) {
	return(rb->BufMask - ringbuffer_used(rb));
	}


/// Add a character to the ringbuffer, and return the 
/// number of characters free in the ring buffer.
/// 
/// If incrementing the write counter makes it equal to 
/// the read counter, that means that we're full.   Drop it on the floor.
int32_t ringbuffer_addchar(RINGBUF* rb, uint8_t c) {
	uint32_t iWritePend = rb->iWrite + 1; 

	// In any case, we're going to save the data.  The decision 
	// is whether or not to advance the pointers.
	rb->Buf[rb->iWrite & rb->BufMask] = c; 

	if ( (iWritePend & rb->BufMask) != (rb->iRead & rb->BufMask) ) {
		rb->iWrite = iWritePend;
		}
	else {
		rb->Dropped++;
		}
	return(ringbuffer_free(rb));
	}

/// Pull a character out of the ringbuffer
/// Return zero if there is nothing in there. 
uint8_t ringbuffer_getchar(RINGBUF* rb) {
	uint8_t c;
	if ( ringbuffer_used(rb) ) { 
		// Get the char, then advance the pointer
		c = rb->Buf[rb->iRead & rb->BufMask];
		rb->iRead += 1;
		return(c);
		}
	else return(0);
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

