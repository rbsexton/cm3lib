// 
// Ringbuffer routines tailored to my particular needs.
// Copyright(C) 2012 Robert Sexton
//

#ifndef __STDINT_H__
#include <stdint.h>
#endif

/// If the counts are both bigger than this, we'll adjust.
#define RB_COUNT_OVERFLOW 0x40000000

typedef struct {
	uint32_t iWrite; 
	uint32_t iRead;  

	uint32_t Dropped;    /// Record dropped characters
	uint32_t ResetCount; /// How many times we  
	
	uint8_t* Buf; // Pointer to the storage area. 
	uint32_t BufSize;  // Length of the storage area 
	uint32_t BufMask;  // Used for masking the index.
	} RINGBUF;

void ringbuffer_init(RINGBUF* , uint8_t*, int);
int32_t ringbuffer_free(RINGBUF*);

int32_t ringbuffer_addchar(RINGBUF*, uint8_t);
uint8_t ringbuffer_getchar(RINGBUF*); 

uint8_t *ringbuffer_getbulkpointer(RINGBUF*); 
int32_t ringbuffer_getbulkcount(RINGBUF*); 
void    ringbuffer_bulkremove(RINGBUF*, int32_t); 

uint32_t ringbuffer_used(RINGBUF*);
int  ringbuffer_reset_count(RINGBUF*);



