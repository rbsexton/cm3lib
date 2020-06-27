///
/// @file wdt.c
/// @brief Watchdog timer management
///
/// Act slowly.  The intent here is to reset a system killed by user error.

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/watchdog.h"

#include "wdt.h"

/// Initialize the watchdog
void WatchdogInit(int initialval) {
    //
    // Check to see if the registers are locked, and if so, unlock them.
    //
    if(WatchdogLockState(WATCHDOG0_BASE) == true) {
        WatchdogUnlock(WATCHDOG0_BASE);
        }

    // Turn on stalling so the debugger doesn't reset the board.
    WatchdogStallEnable(WATCHDOG0_BASE);

    //
    // Initialize the watchdog timer to an initial value of 10s
    //
    WatchdogReloadSet(WATCHDOG0_BASE, initialval);

    // Enable the reset.
    WatchdogResetEnable(WATCHDOG0_BASE);

    // Enable the interrupt
    WatchdogIntEnable(WATCHDOG0_BASE);

    // Enable the watchdog timer.
    WatchdogEnable(WATCHDOG0_BASE);

    // Lock it.
    WatchdogUnlock(WATCHDOG0_BASE);

    }

/// Refresh the watchdog.  Be sure and pass in constants that can't be corrupted.
void WatchdogKick(int value) {
    WatchdogUnlock(WATCHDOG0_BASE);
    WatchdogReloadSet(WATCHDOG0_BASE, value);
    WatchdogUnlock(WATCHDOG0_BASE);
    }


/// Watchdog ISR
///
/// At the present time, this is just a stub.  In case of a hardfault,
/// this won't be able to get any attention because hardfault is high priority

void WatchdogHandler() {

    while (1) { ; }
    }
    
    