@*************************************************************
@ Load up the designated user app and give it control of the
@ system.  The user app is a Cortex-M executable that will
@ take complete ownership of the system after the startup code
@ does the hardware initialization.
@
@ In other contexts, this could be considered a normal part of 
@ operating a bootloader.
@

.syntax unified
.section .text 

@*************************************************************
@ LaunchUserApp(uint32_t *appaddr, uint32_t *runtimep) 
@
@ Loads up the stack pointer and the initial PC from memory
@ and starts things off.   Passes in the runtime pointer.
@ This will abandon any stack allocated in memory for the 
@ supervisor, so its a bit wasteful. 
@
@	R0: Starting address of the user app in memory.
@	R1: Pointer to runtime data to share with forth.
@	No return 

@*************************************
.global LaunchUserApp
LaunchUserApp:
  cpsid i 
  ldr r2, [ r0, #0 ] /* Thats the stack pointer */
  mov sp, r2
  ldr r2, [ r0, #4 ] /* The initial PC */
  mov r0, r1         /* Put the RT Link in the right spot */
  cpsie i
	bx  r2

@*************************************************************
@ LaunchUserAppThread(uint32_t *appaddr, uint32_t *runtimep)
@
@ Loads up the stack pointer and the initial PC from memory
@ and starts things off.  Switches to thread mode.
@ Passes in the runtime pointer.
@ 
@ Compatible with Cortex-M0
@
@	R0: Starting address of the user app in memory.
@	R1: Pointer to runtime data to share with forth.
@	Does not return 

@*************************************
.global LaunchUserAppThread
LaunchUserAppThread:
	cpsid i 
	ldr r2, [ r0, #0 ] /* Thats the stack pointer */
	msr psp, r2
	movs r3, # 2
	msr control, r3
	isb					// Per the Arm Docs.

	ldr r2, [ r0, #4 ] /* The initial PC */
	mov r0, r1         /* Put the RT Link in the right spot */
	cpsie i
	bx  r2

@*************************************************************
@ void LaunchUserAppNoSP(uint32_t *appaddr, uint32_t *runtimep) 
@
@ Loads up the initial PC from memory and starts things off.
@ Use the existing stack pointer.
@
@	R0: Starting address of the user app in memory.
@	R1: Pointer to runtime data to share with forth.
@	Does not return 

@*************************************
.global LaunchUserAppNoSP

LaunchUserAppNoSP:
  ldr r2, [ r0, #4 ] /* The initial PC */
  mov r0, r1  
	bx  r2

@*************************************************************
@ LaunchUserAppUpdateNVIC(uint32_t *appaddr, uint32_t *runtimep) 
@
@ Loads up the initial PC from memory and starts things off.
@ Adjust the VTOR register in the NVIC so that the app can use
@ the interrupt vectors.
@
@	R0: Starting address of the user app in memory.
@	R1: Pointer to runtime data to share with forth.
@
@	Does not return 
@
@ The Cortex-M0 has no VTOR register.
@

@*************************************
.global LaunchUserAppUpdateNVIC

LaunchUserAppUpdateNVIC:
	cpsid i 
	ldr r2, [ r0, #0 ] /* Stack pointer */
  mov sp, r2

	ldr r2, =0xE000ED08 /* VTOR */
	str r0, [ r2, #0 ]
                 
  ldr r2, [ r0, #4 ] /* Initial PC */
  mov r0, r1 /* Runtime link */                  

  cpsie i 
  bx r2
 .end

