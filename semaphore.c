/// @file semaphore.
/// 
/// LDREX/STREX based semaphore operators

#include <stdint.h>


// Note that Interrupts/Excpetions may clear the exclusive monitor, so just retry.
void sem_change(uint32_t *sem, int32_t delta) {
	__asm(	"L: ldrex	r2, [ %[sem], #0 ]\n"
	       	"add		r2, %[delta]\n"
			"strex		r3 , r2, [ %[sem] ]\n" 
			"cmp		r3, #1\n"
			"bne		L\n" : :
			  [sem] "r" (sem), [delta] "r" (delta) : "r2", "r3", "cc"
		);

	}