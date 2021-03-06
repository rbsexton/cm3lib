// Catch a fault, Write a report into a buffer.
// Requires a dedicated stack so that we can
// Copyright 2009 Robert Sexton


#include <stdint.h>
#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_uart.h>
#include <inc/hw_ints.h>
#include <inc/hw_nvic.h>
#include <driverlib/uart.h>

#include "utils/ustdlib.h"

// declare these static so that they have dedicated storage,
// and we don't have to do any stack operations

uint8_t  fh_mmsr;
uint8_t  fh_bfsr;
uint16_t fh_ufsr;
uint32_t fh_hfsr;
uint32_t fh_dfsr;
uint32_t fh_afsr;

uint32_t fh_bfar;

// Harvest a few key registers
uint32_t fh_oldpc;
uint32_t fh_oldsp;
uint32_t fh_oldlr;
uint32_t fh_xpsr;  // Status register

// This is a one way trip.

// void HardFaultHandler(void) {

__attribute__ ((naked)) void  HardFaultHandler(void)   {

    // while( 1 ) { ; } // Spin

    __asm__ __volatile__ (
        ".thumb \n\t"
        ".thumb_func \n\t"
        "@ harvest the registers \n\t"

        "MRS	R0,XPSR\n\t"
        "LDR	R1, =fh_xpsr\n\t"
        "STR	R0, [R1,#0]\n\t"

        "@ Determine which stack\n\t"
        "TST 	LR,#0x04\n\t"
        "ITE EQ\n\t"
        "MRSEQ R0, MSP\n\t"
        "MRSNE R0, PSP\n\t"

        "LDR   R2, =fh_oldpc\n\t"
        "LDR   R1, [R0, #24]\n\t"
        "STR   R1, [R2,#0]\n\t"

        "LDR   R2, =fh_oldlr\n\t"
        "LDR   R1, [R0, #20]\n\t"
        "STR   R1, [R2,#0]\n\t"

        "LDR   R2, =fh_oldsp\n\t"
        "ADD   R0, #32\n\t"
        "STR   R0, [R2,#0]\n\t"

        : : : "r0", "r1", "r2" );

    // Some Naked Constants....
    fh_bfar = *( (uint32_t *) 0xe000ed38);

    fh_mmsr = *( (uint8_t *)  0xe000ed28);
    fh_bfsr = *( (uint8_t *)  0xe000ed29);
    fh_ufsr = *( (uint16_t *) 0xe000ed2a);
    fh_hfsr = *( (uint32_t *) 0xe000ed2c);
    fh_dfsr = *( (uint32_t *) 0xe000ed30);
    fh_afsr = *( (uint32_t *) 0xe000ed3c);


    __asm__ __volatile__ (
        ".thumb \n\t"
        ".thumb_func \n\t"
        "b FaultISRnice\n\t"
        ".align\n\t"
        : : );

    return; // Never happens

    }

char buffer[64];

void FaultISRnice(void) {
    // strcpy(buffer,buf0);
    // for ( p = buffer; *p != '\0'; p++) UARTCharPut(UART0_BASE,*p);

    usprintf(buffer, "Fault %d\r\n", (fh_xpsr & 0x1FF));

    for ( char* p = buffer; *p != '\0'; p++ ) UARTCharPut(UART0_BASE, *p);

    usprintf(buffer, "XFSR: 0x%08x PC: 0x%08x ", fh_xpsr, fh_oldpc);

    for ( char* p = buffer; *p != '\0'; p++ ) UARTCharPut(UART0_BASE, *p);

    usprintf(buffer, "SP: 0x%08x LR: 0x%08x\r\n", fh_oldsp, fh_oldlr);

    for ( char* p = buffer; *p != '\0'; p++ ) UARTCharPut(UART0_BASE, *p);

    usprintf(buffer, "MMSR: %02x BFSR: %02x\r\n", fh_mmsr, fh_bfsr);

    for ( char* p = buffer; *p != '\0'; p++ ) UARTCharPut(UART0_BASE, *p);

    usprintf(buffer, "UFSR: %02x HFSR: %02x\r\n\0", fh_ufsr, fh_hfsr);

    for ( char* p = buffer; *p != '\0'; p++ ) UARTCharPut(UART0_BASE, *p);

    usprintf(buffer, "DFSR: %02x AFSR: %02x\r\n\0", fh_dfsr, fh_afsr);

    for ( char* p = buffer; *p != '\0'; p++ ) UARTCharPut(UART0_BASE, *p);

    // See if there's good data in there
    if ( fh_bfsr & 0x80 ) {
        usprintf(buffer, "BFAR: %02x\r\n\0", fh_bfar);

        for ( char* p = buffer; *p != '\0'; p++ ) UARTCharPut(UART0_BASE, *p);
        }

    // Spin forever, wait for help.
    while(1) { ; }

    }

