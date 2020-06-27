/// @file panic.c
/// @brief Tools for handling panics
///
/// Copyright 2014 Robert Sexton


#include <stdint.h>

uint32_t panic_registers[17];

#define PANIC_PSR    64
#define PANIC_R15_PC 60
#define PANIC_R14_LR 56
#define PANIC_R13_SP 52

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

        "@ Snapshot R0 so we get it too\n\t"
        "PUSH { R0 }\n\t"
        "LDR R0, =panic_registers\n\t"
        "STMIA R0, { r0-r12 }\n\t"

        "@ Start filling things in\n\t"
        "POP { R1 }\n\t"
        "STR R1, [ R0, #0]\n\t"

        "MOV R1, SP\n\t"
        "ADD R1, # 4\n\t"
        "STR R1, [ R0, #52 ]\n\t"

        "STR LR, [ R0, #60 ]\n\t"

        "MRS R1, PSR\n\t"
        "STR R1, [ R0, #64 ]\n\t"

        : : : "r0", "r1", "r2" );
    }

