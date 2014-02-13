// 
// Macros of use in developing fault handlers.
// Copyright 2014 Robert Sexton.
//
// This software has NO WARRANTY.
// I hereby place it into the public domain for any use you see fit.
//
// Largely inspired by Joseph Yiu's Cortex-M3 book.   Saves all of the resgisters to 
// a holding area so you can make a nice rigister dump.
//
// GCC Register syntax.
// Assumes that you have a 17-word array available to hold 
// the register dump.
// Include this in another function that does something nice with the 
// information.

#define FH_REGISTER_SAVE __asm__ __volatile__ ( \
   ".thumb \n\t" \
   ".thumb_func\n\t" \
   " @ Pull PC from the right stack\n\t" \
   "TST 	LR,#0x04\n\t" \
   "ITE EQ         \n\t" \
   "MRSEQ R1, MSP  \n\t" \
   "MRSNE R1, PSP  \n\t" \
   \
   "push {r4-r7}\n" \
   \
   "MRS	R1,XPSR             \n\t" \
   "STR	R1, [R1,#64]        \n\t" \
   \
   "ldr	R0, =regdump\n\t" \
   \
   " @ Now move the dump pointer over the first four, then back\n\t" \
   "ADD R0, #16                  \n\t" \
   "STMIA.W  R1, { r4-r11 }      @ Get the others\n\t " \
   "@ That Completes r4-r11\n\t" \
   \
   "SUB    R0, #16         \n" \
   "LDMIA	R1, {r4-r7}\n\t" \
   "STMIA  R0, {r4-r7}\n\t" \
   "@ Now we have r0-r11\n\t" \
   \
   "LDR R2, [ R1, #16] @ r12\n\t" \
   "STR R2, [ R0, #48]      \n\t" \
   \
   "LDR R2, [ R1, #20] @ r14/lr\n\t" \
   "STR R2, [ R0, #56]         \n\t" \
   \
   "LDR R2, [ R1, #24] @ r15/pc\n\t" \
   "STR R2, [ R0, #60]         \n\t" \
   \
   "@ SP during fault\n\t" \
   "ADD R1, #32\n\t" \
   "STR   R1, [ R0, #52] @ r13/sp\n\t" \
   \
   "pop {r4-r7}\n" \
   : : : "r0","r1","r2", "memory" );
		
