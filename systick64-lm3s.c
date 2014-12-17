/// @file systick64-lm3s.c
/// @brief A 64-Bit Systick timer 
/// Accessed via function.
/// This must be called at a very high IRQ level in order to prevent some interesting
/// failures.

#include <stdint.h> 
#include <stdbool.h> 

#include "systick64.h"

uint64_t SysTickMSVal64 = 0; 

void SysTickMSUpdate64(unsigned ms) {
	SysTickMSVal64 += ms;
	}

/// Here is a generic gadget.
/// This is just easier in assembly.  Take a pointer to 32-bit stuff 
/// so the compiler finds the address.
uint64_t get64BitCounter(uint32_t *counter) {
	// We have to do a read of the top half, then the bottom, then the second to be safe.
	__asm( "	mov r3, r0\n"
		   "1:  ldr r1, [ r3, #4 ] @ Top Half\n"
		   "    ldr r0, [ r3, #0 ] @ Bottom Half\n"
		   "    ldr r2, [ r3, #4 ] @ Make sure top half is unchanged\n"
		   "    cmp r1, r2 @ If the ISR snuck in there, try again.\n"
		   "    bne $1b\n"
		   " bx lr\n" );
	return(1); // Never happens.
	}

/// Non-privileged/protected.
uint64_t getSysTickMS64() {
	return(get64BitCounter( (uint32_t *) &SysTickMSVal64 ) );  
	}

/// Lots of Systick code uses 32-bit numbers.
uint32_t getSysTickMS32() {
	uint32_t *res = (uint32_t *) &SysTickMSVal64;
	return( res[0] );
	}
