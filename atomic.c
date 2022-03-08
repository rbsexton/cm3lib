/// @file atomic.c
///
/// @brief LDREX/STREX based semaphore operators
/// ARM App note DAI0321A says that you need memory barriers 
/// after writing via strex, or before clearing a lock.
/// These functions can be used either way, so include a
/// memory barrier before and after the exclusive operators.
///
/// Compile this with optimization, otherwise you'll get a bunch 
/// of useless stack operations.

#include <stdint.h>

/// @brief Do an atomic add.
/// @return the new value
/// @param *sem pointer to the underlying value
/// @param delta how much to add
// Note that Interrupts/Exceptions may clear the exclusive monitor, so just retry.
// This function will always succeed.
// Return the result of add.
//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// If you compile this without optimization you will get a terrible result.
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
int32_t atomic_add(uint32_t *sem, int32_t delta) {
    int32_t result;
    __asm( "  dmb   \n" 
          "1: ldrex  %[result], [ %[sem], #0 ]\n\t"
              "add   %[result], %[delta]\n\t"
              "strex r3 , %[result], [ %[sem] ]\n\t"
              "cmp   r3, #1\n\t"
              "bne   b1\n"
              "dmb   \n" // Required by Architecture. (DAI0321A)
            : [result] "=&r" (result) :
            [sem] "r" (sem), [delta] "r" (delta) : "r3" );

    return(result);
    }
    
int32_t atomic_mask_or(uint32_t *sem, uint32_t mask) {
      int32_t result;
      __asm("   dmb   \n" 
            "1: ldrex  %[result], [ %[sem], #0 ]\n\t"
                "orr   %[result], %[mask]\n\t"
                "strex r3 , %[result], [ %[sem] ]\n\t"
                "cmp   r3, #1\n\t"
                "bne   b1\n"
                "dmb   \n" // Required by Architecture. (DAI0321A)
              : [result] "=&r" (result) :
              [sem] "r" (sem), [mask] "r" (mask) : "r3" );

      return(result);
      }

int32_t atomic_mask_and(uint32_t *sem, uint32_t mask) {
      int32_t result;
      __asm("   dmb   \n" 
            "1: ldrex  %[result], [ %[sem], #0 ]\n\t"
                "and   %[result], %[mask]\n\t"
                "strex r3 , %[result], [ %[sem] ]\n\t"
                "cmp   r3, #1\n\t"
                "bne   b1\n"
                "dmb   \n" // Required by Architecture. (DAI0321A)
              : [result] "=&r" (result) :
              [sem] "r" (sem), [mask] "r" (mask) : "r3" );

      return(result);
      }
    

    
    
