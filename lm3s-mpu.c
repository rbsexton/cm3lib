// Stellaris/TI MPU setup for use with user task management.
// Core concept - Handler Mode owns the slots, and will load 
// up slots 2 - 5 on demand. 

// 0: r/o User access for all of memory.
// 1: r/w Access to user memory.

// User Slots go here.
 
// 6: r/o User access for SCS/NVIC
// 7: r/o User access for the Supervisor Data.

// Option A -  Default R/W for all -
// 0: R/W all,                             6: NVIC Lockout, 7: Supervisor mem lockout
// Option B - Default R/O for all - 
// 0: R/O for all, 1: R/W for user memory, 6: NVIC Lockout, 7: Supervisor mem lockout
//
// Option a is better, becasue the periphals and bit-band regions don't need to be
// explicitly called out.
//
//*****************************************************************************
// MPU Setup for use  
//*****************************************************************************
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/mpu.h"
#include "driverlib/rom.h"

void mpu_init() { 

        // --------------------------- Supervisor Slots --------------------------
        // Overlay, Prv R/W, User R/W region for everything.
        MPURegionSet(0, 0, MPU_RGN_SIZE_4G| MPU_RGN_PERM_EXEC |
                     MPU_RGN_PERM_PRV_RW_USR_RW | MPU_RGN_ENABLE);


        // --------------------------- User Slots --------------------------
		// Templates - R/O fences.

        MPURegionSet(2, 0x2002ff80, MPU_RGN_SIZE_32B | MPU_RGN_PERM_NOEXEC |
		     MPU_RGN_PERM_PRV_RW_USR_RO);
        MPURegionSet(3, 0x2002ffa0, MPU_RGN_SIZE_32B | MPU_RGN_PERM_NOEXEC |
		     MPU_RGN_PERM_PRV_RW_USR_RO);
        MPURegionSet(4, 0x2002ffc0, MPU_RGN_SIZE_32B | MPU_RGN_PERM_NOEXEC |
		     MPU_RGN_PERM_PRV_RW_USR_RO);
        MPURegionSet(5, 0x2002ffe0, MPU_RGN_SIZE_32B | MPU_RGN_PERM_NOEXEC |
		     MPU_RGN_PERM_PRV_RW_USR_RO);

        // --------------------------- Supervisor Slots --------------------------
        // Block off NVIC write.
        MPURegionSet(6, 0xe0000000, MPU_RGN_SIZE_512K | MPU_RGN_PERM_NOEXEC |
		     MPU_RGN_PERM_PRV_RW_USR_RO | MPU_RGN_ENABLE);
        // Block off the top 56k of supervisor memory.
        MPURegionSet(7, 0x20000000,
		     MPU_RGN_SIZE_128K | MPU_RGN_PERM_NOEXEC |
		     MPU_RGN_PERM_PRV_RW_USR_RO | MPU_RGN_ENABLE);
        // Setup things for priv access by default.
        ROM_MPUEnable(MPU_CONFIG_PRIV_DEFAULT);
       
    	// I don't recall if we also have to enable the regions. 
}
