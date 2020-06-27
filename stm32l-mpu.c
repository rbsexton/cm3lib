// @ brief mpu-cmsis.c
// Setting up the MPU via the CMSIS Calls
//
// The exact choice of bufferable, TEX, etc appears to be target specific.
// These choices are for the STM32L with a dual-stack setup.
//
// The Luminary Micro/Stellaris stuff is way easier to work with than CMSIS.

#include <stdint.h>

#include "stm32l1xx.h"
#include "core_cm3.h"

#define MPU_SIZE_4G                              (0x1FUL << MPU_RASR_SIZE_Pos)
#define MPU_SIZE_4K                              (0x0BUL << MPU_RASR_SIZE_Pos)
#define MPU_SIZE_32B                             (0x08UL << MPU_RASR_SIZE_Pos)

#define MPU_PRW_URW  (0x03UL << MPU_RASR_AP_Pos)
#define MPU_PRW_URO  (0x02UL << MPU_RASR_AP_Pos)

#define RASR_Entry_Enable 1

// Here is a default table.  Note that we just skip the
// Region select register and whack it all in.
// This also includes templates
//
// Regions
// 0 R/W overlay for the whole world
// Slots for the user-mode task to use.
// 6 R/O user overlay for the supervisor memory
// 7 R/W user overlay for a shared memory area
// One gotcha - If you don't load up a valid area, it'll use the Region selector
// register.  That may not be what you want, so only load valid, enabled regions.
// In this case, just use duplicates for filler.
uint32_t const mpudefaults [8][2] = {
        {          0 | MPU_RBAR_VALID_Msk | 0, MPU_PRW_URW | MPU_SIZE_4G | RASR_Entry_Enable },
        {          0 | MPU_RBAR_VALID_Msk | 1, MPU_PRW_URW | MPU_SIZE_4G  },
        { 0x20000000 | MPU_RBAR_VALID_Msk | 2, MPU_PRW_URO | MPU_SIZE_4K  },
        { 0x20000000 | MPU_RBAR_VALID_Msk | 3, MPU_PRW_URO | MPU_SIZE_4K  },
        { 0x20000000 | MPU_RBAR_VALID_Msk | 4, MPU_PRW_URO | MPU_SIZE_4K  },
        { 0x20000000 | MPU_RBAR_VALID_Msk | 5, MPU_PRW_URO | MPU_SIZE_4K  },
        { 0x20000000 | MPU_RBAR_VALID_Msk | 6, MPU_PRW_URO | MPU_SIZE_4K | RASR_Entry_Enable  },
        {          0 | MPU_RBAR_VALID_Msk | 7, MPU_PRW_URW | MPU_SIZE_4G  }

    };


void mpu_setup() {

    uint32_t i;

    /* Disable MPU */
    MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;

    for (i = 0; i < 8; i++) {
        MPU->RBAR = mpudefaults[i][0];
        MPU->RASR = mpudefaults[i][1];
        }

    /* Enable MPU */
    MPU->CTRL |= MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_ENABLE_Msk;

    }


