/// @file semaphore.
/// 
/// LDREX/STREX based semaphore operators

#include <stdint.h>


// Note that Interrupts/Excpetions may clear the exclusive monitor, so just retry.
// This function will always succeed.  
// Return the result of add.
int32_t atomic_add(uint32_t *sem, int32_t delta) {
	int32_t result, scratch;
	__asm(	"L: ldrex	%[result], [ %[sem], #0 ]\n\t"
	       	"add		%[result], %[delta]\n\t"
			"strex		%[scratch] , %[result], [ %[sem] ]\n\t" 
			"cmp		%[scratch], #1\n\t"
			"bne		L\n"
			: [result] "=&r" (result), [scratch] "=r" (scratch) :
			  [sem] "r" (sem), [delta] "r" (delta) : );

	return(result);
	}
