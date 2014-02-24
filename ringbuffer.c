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
	rb->freecount = size;
	}


// Remember that it cannot fill up.
// int ringbuffer_free(RINGBUF* rb) {
// 	return(rb->bufmask -  (rb->next_write - rb->next_read) );

int ringbuffer_free(RINGBUF* rb) {
	return(rb->freecount);
	}

// int ringbuffer_used(RINGBUF* rb) {
//	return(rb->next_write - rb->next_read);
// 	}
// int ringbuffer_notempty(RINGBUF* rb) {
//	return(rb->next_write != rb->next_read);
// 	}

int ringbuffer_used(RINGBUF* rb) {
	return(rb->bufsize - rb->freecount);
	}

// Reset the two pointers to prevent wrap.
// critial locking must come from outside.
void ringbuffer_reset(RINGBUF* rb) {
	rb->next_write -= rb->next_read;
	rb->next_read  -= rb->next_read;
	}

// Add a character to a buffer, and return the number
// of spaces available.
int ringbuffer_addchar(RINGBUF* rb, uint8_t c) {
	if ( rb->freecount > 1 ) {
		rb->buf[rb->next_write & rb->bufmask] = c;
		(rb->freecount)--;
		rb->next_write += 1; 
		}
	else { // It gets complicated.  Steal one
		rb->buf[rb->next_write & rb->bufmask] = c;
		rb->next_write += 1;
		rb->next_read += 1;
		rb->dropped += 1;
		// No change in freecount
		}
	return(rb->freecount);
	}
// Do the converse of add.  
uint8_t ringbuffer_getchar(RINGBUF* rb) {
	uint8_t c;
	// If empty, lie rather than make it worse.
	// otherwise, theres more room now.
	if ( rb->freecount >= rb->bufsize ) { return(0); }
	else { (rb->freecount)++; }

	// Get the char, then advance the pointer
	c = rb->buf[rb->next_read & rb->bufmask];
	rb->next_read += 1;
	return(c);
	}


