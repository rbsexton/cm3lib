/// @file locker.c
/// @brief Bitband-based locking
/// @author Robert Sexton
/// Advisory locking functions that use the Cortex-M3 bit-banding scheme 
///

// Cast to turn and address into an implicit access, and force it volatile
#define VWRAP(addr) (*((volatile unsigned long *)(addr)))

/// Convert an address, bit pair to the bit-banded address
/// @param Address of the word
/// @param Bit number
/// @return the calculated address
unsigned long bitbanded_address(unsigned long addr,int bit) {

	return( ((addr & 0xf0000000 ) |\
			         0x02000000 ) |\
		   ( ( addr &  0x000fffff) << 5 ) |\
		   ((bit) << 2) );
	}
                            
/// Try to get an advisory lock.
/// @param Address of the word to use for locking
/// @param Bit number to use for locking purposes
/// @return
/// - 1: Success
/// - 0: Failure
int get_bitbanded_lock(unsigned long lockaddr, int bit) {

	unsigned mask, locked;
	unsigned bbptr;  // This will get wrapped in a cast.
	
	bbptr = bitbanded_address(lockaddr,bit);
	
	if ( VWRAP(lockaddr) != 0 ) return(0); // Somebody has it already
	
	// Set the lock
	VWRAP(bbptr) = 1;
	
	locked = VWRAP(lockaddr);

	mask = ~(1 << bit);
	
	// Mask out our bit and look for others.
	if ( (locked & mask) != 0 ) { // FAIL!
		VWRAP(bbptr) = 0; // Clear it.
		return(0);
		}
	else { // Success
		return(1);
		}

	}

/// Release the lock.  There is no need for checking, just do it.
/// @param Address of the word to use for locking
/// @param Bit number to use for locking purposes
/// @return
/// - 1: Success
/// - 0: Failure
void release_bitbanded_lock(unsigned long lockaddr, int bit) {
	unsigned bbptr;  // This will get wrapped in a cast.
	bbptr = bitbanded_address(lockaddr,bit);
	VWRAP(bbptr) = 0;
	}		
	
