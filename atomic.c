/// @file semaphore.
/// 
/// LDREX/STREX based semaphore operators

#include <stdint.h>

// Note that Interrupts/Exceptions may clear the exclusive monitor, so just retry.
// This function will always succeed.  
// Return the result of add.
// 
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// If you compile this without optimization you will get a terrible result.
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
int32_t atomic_add(uint32_t *sem, int32_t delta) {
	int32_t result;
	__asm(	"L: ldrex	%[result], [ %[sem], #0 ]\n\t"
	       	"add		%[result], %[delta]\n\t"
			"strex		r3 , %[result], [ %[sem] ]\n\t" 
			"cmp		r3, #1\n\t"
			"bne		L\n"
			"dmb		\n" // Required by Architecture. (DAI0321A)
			: [result] "=&r" (result) :
			  [sem] "r" (sem), [delta] "r" (delta) : "r3" );

	return(result);
	}
