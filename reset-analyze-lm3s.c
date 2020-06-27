/// \file reset-lm3s.c
/// \brief Sample Code for Analyzing reset causes
///

#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"

unsigned long ulVal;

ulVal =  HWREG(SYSCTL_RESC);

if ( ulVal & SYSCTL_RESC_POR ) {
    HWREG(SYSCTL_RESC) &=  ~(SYSCTL_RESC_POR);
    UARTprintf("Reset Cause: Power On\n");
    }

if ( ulVal & SYSCTL_RESC_EXT ) {
    HWREG(SYSCTL_RESC) &=  ~(SYSCTL_RESC_EXT);
    UARTprintf("Reset Cause: Ext\n");
    }

