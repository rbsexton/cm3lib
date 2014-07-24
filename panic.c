/// @file panic.c
/// @brief Tools for handling panics
///
/// Copyright 2014 Robert Sexton


#include <stdint.h>

uint32_t panic_registers[17]; 

#define PANIC_APSR   (16*4)
#define PANIC_R15_PC (15*4)
#define PANIC_R14_LR (14*4)
#define PANIC_R13_SP (13*4)

// A function that makes a register snapshot.
// It assumes that the link register is stacked
// Note that this is not designed for use across
// exceptions, so there is no code to do stack ID.
// Not that the assumption here is that this will be called
// before anything else, so it can assume that there is a 
// stacked LR
void panic_snapshot() {

		 __asm__ __volatile__ (
                ".thumb \n\t"
                ".thumb_func \n\t"
                "@ harvest the registers \n\t"

				"@ Save a copy of R2 so we can bootstrap\n\t"
				"PUSH { R2 }\n\t"
				"LDR R2, =panic_registers\n\t"
				"STMIA R2, { r0-r1 }\n\t"
				"MOV r0, r2\n\t"
				"POP { R2 }\n\t"

				"STMIA R0, { r2-r12 }\n\t"
				"MOV R1, SP\n\t"
				"ADD R1, # 4\n\t"
				"STR R1, [ R0, #0 ]\n\t"
				
				"MRS R1, APSR\n\t"
				"STR R1, [ R0, #12 ]\n\t" 
				
	            : : : "r0","r1","r2" );
	}

