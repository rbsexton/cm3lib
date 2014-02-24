// 
// Ringbuffer routines tailored to my particular needs.
// Copyright(C) 2012 Robert Sexton
//
// Basic Model - Struct with external storage
// - No interrupt protection.  Thats externally supplied.
// - Dynamically created so that the same code maintains multiple buffers.
// - Counted contents so you can put in a big chunk.
// - Requires power of two sizing.
// There are edge conditions when its full.
//
// Unit tests worth writing:
// Put a char in, get the same one out.   Repeat until wrap
// Put three in, then pull one out.  Add one.  repeat util wrap.
//
// Ringbuffer, v2.   Make things more robust to better live in a world
// without locking.   Two pointers, reader and writer.  
// They both get used modulo the pointer size.   Equality means empty.
// 
// Do not permit the read and write pointer to ever wrap.   We're going to drop
// characters when things overflow.   The cost of adjusting pointers to handle that
// case is just too high.
//
// ringbuffer_notempty()
// 

// #include "ringbuffer.h"
#include <stdint.h>

#include "ringbuffer.h"

void ringbuffer_init(RINGBUF* rb, uint8_t* buf, int size) {
	rb->next_read=0;   
	rb->next_write=0;
	rb->dropped=0;
	rb->buf = buf;
	rb->bufsize = size;
	rb->bufmask = size - 1;
	}

// Remember that it is not allowed to fill up.
int ringbuffer_free(RINGBUF* rb) {
	return(rb->bufmask -  (rb->next_write - rb->next_read) );
	}

// There used to be a not-empty check, but it turns out that
// ringbuffer_used is a bit faster after compilation is over.
// Used is just a subtract, so the conditional is free.	
int ringbuffer_used(RINGBUF* rb) {
	return(rb->next_write - rb->next_read);
	}

// Reset the two pointers to prevent wrap.
// critial locking must come from outside.
void ringbuffer_reset(RINGBUF* rb) {
	rb->next_write -= rb->next_read;
	rb->next_read  -= rb->next_read;
	}

// Add a character to a buffer, and return the number
// of spaces available.   Note that pre-emption
// results in false full indication.  
int ringbuffer_addchar(RINGBUF* rb, uint8_t c) {
	int free = ringbuffer_free(rb);
	if ( free > 1 ) {
		rb->buf[rb->next_write & rb->bufmask] = c;
		rb->next_write += 1;
		return(free -1); 
		}
	else { // If no space, drop it on the floor.
		return(free);
		}
	}
	
// Do the converse of add.  
uint8_t ringbuffer_getchar(RINGBUF* rb) {
	uint8_t c;
	// If empty, lie rather than make it worse.
	// otherwise, theres more room now.
	if ( ringbuffer_used(rb) ) { 
		// Get the char, then advance the pointer
		c = rb->buf[rb->next_read & rb->bufmask];
		rb->next_read += 1;
		return(c);
		}
	else return(0);
	}


